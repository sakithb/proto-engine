#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cgltf.h"
#include "cglm/struct.h"
#include "stb_image.h"
#include "memutils.h"
#include "model_fmt.h"

void model_process(const char *path);
void model_process_nodes(cgltf_data *data, struct header_fmt *header, const char *dir, FILE *f);
static inline void model_process_load_image(cgltf_image *img, struct texture_fmt *tex, const char *dir, FILE *f);

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		model_process(argv[i]);
	}

	return 0;
}

void model_process(const char *path) {
	size_t dir_len = 0;

	char *basename = strrchr(path, '/');
	if (basename == NULL) {
		basename = (char*)path;
	} else {
		dir_len = basename - path;
		basename += 1;
	}

	char dir[dir_len + 1];
	strncpy(dir, path, dir_len);
	dir[dir_len] = '\0';

	char *period = strrchr(path, '.');
	if (period == NULL) {
		period = basename + strlen(basename) - 1;
	}

	char filepath[period - basename + 21];
	strncpy(filepath, "assets/models/", 14);
	strncpy(filepath + 14, basename, period - basename);
	strcpy(filepath + 14 + (period - basename), ".model");

	FILE *f = fopen(filepath, "wb");
	if (f == NULL) {
		fprintf(stderr, "Error opening file %s\n", filepath);
		abort();
	}

	struct header_fmt header = {0};
	fwrite(&header, sizeof(struct header_fmt), 1, f);

	cgltf_options options = {0};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, path, &data);
	if (result != cgltf_result_success) {
		fprintf(stderr, "cgltf parse error %s\n", path);
		abort();
	}

	result = cgltf_load_buffers(&options, data, path);
	if (result != cgltf_result_success) {
		fprintf(stderr, "cgltf load buffers error %s\n", path);
		abort();
	}

	model_process_nodes(data, &header, dir, f);

	cgltf_free(data);

	fseek(f, 0, SEEK_SET);
	fwrite(&header, sizeof(struct header_fmt), 1, f);

	fclose(f);
}

#define load_attribute(accessor, comps, type, dst) { \
	int n = 0; \
	type *buffer = (type*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset/sizeof(type) + accessor->offset/sizeof(type); \
	for (int i = 0; i < accessor->count; i++) { \
		for (int j = 0; j < comps; j++) { \
			dst[comps * i + j] = buffer[n + j];\
		} \
		n += (int)(accessor->stride/sizeof(type));\
	}\
}

void model_process_nodes(cgltf_data *data, struct header_fmt *header, const char *dir, FILE *f) {
    size_t meshes_num = 0, attrs_num = 0, indcs_num = 0, mats_num = 0, texs_num = 0;
	for (int i = 0; i < data->nodes_count; i++) {
		cgltf_node *node = &data->nodes[i];
		cgltf_mesh *mesh = node->mesh;
		if (mesh == NULL) continue;

		for (int j = 0; j < mesh->primitives_count; j++) {
			if (mesh->primitives[j].type != cgltf_primitive_type_triangles) {
				fprintf(stderr, "only triangles are supported\n");
				abort();
			}

			if (mesh->primitives[j].indices == NULL) {
				fprintf(stderr, "unindexed attributes are not supported\n");
				abort();
			} else if (mesh->primitives[j].indices->component_type != cgltf_component_type_r_16u &&
					   mesh->primitives[j].indices->component_type != cgltf_component_type_r_32u) {
				fprintf(stderr, "unsupported index type\n");
				abort();
			}

			cgltf_accessor *positions = NULL;
			for (int k = 0; k < mesh->primitives[j].attributes_count; k++) {
				if (mesh->primitives[j].attributes[k].type == cgltf_attribute_type_position) {
					positions = mesh->primitives[j].attributes[k].data;
					break;
				}
			}

			if (positions == NULL) {
				fprintf(stderr, "position attributes are required\n");
				abort();
			} else if (positions->type != cgltf_type_vec3 || positions->component_type != cgltf_component_type_r_32f) {
				fprintf(stderr, "unsupported vertex attribute position type\n");
				abort();
			}

			meshes_num++;
			attrs_num += positions->count;
			indcs_num += mesh->primitives[j].indices->count;

			if (mesh->primitives[j].material != NULL && mesh->primitives[j].material->has_pbr_metallic_roughness) {
				if (mesh->primitives[j].material->pbr_metallic_roughness.base_color_texture.texture != NULL) {
					texs_num++;
				}

				mats_num++;
			}
		}
	}

	assert(meshes_num > 0);
	assert(indcs_num > 0);
	assert(attrs_num > 0);

	size_t attrs_size = attrs_num * sizeof(struct attr_fmt);

	struct mesh_fmt *meshes = mallocs(meshes_num * sizeof(struct mesh_fmt));
	struct attr_fmt *attrs = mallocs(attrs_size);
	uint32_t *indcs = mallocs(indcs_num * sizeof(uint32_t));
	struct material_fmt *mats = mallocs(mats_num * sizeof(struct material_fmt));
	struct texture_fmt *texs = mallocs(texs_num * sizeof(struct texture_fmt));

	float *attrs_buf = mallocs(attrs_size);

	size_t meshes_i = 0, attrs_i = 0, indcs_i = 0, mats_i = 0, texs_i = 0;

	for (int i = 0; i < data->nodes_count; i++) {
		cgltf_node *node = &data->nodes[i];
		cgltf_mesh *mesh = node->mesh;
		if (mesh == NULL) continue;

		for (int j = 0; j < mesh->primitives_count; j++) {
			cgltf_primitive prim = mesh->primitives[j];
			struct mesh_fmt *mesh_o = &meshes[meshes_i];

			cgltf_accessor *positions = NULL, *normals = NULL, *uvs = NULL;

			for (int k = 0; k < prim.attributes_count; k++) {
				if (prim.attributes[k].type == cgltf_attribute_type_position) {
					positions = prim.attributes[k].data;
				} else if (prim.attributes[k].type == cgltf_attribute_type_normal) {
					normals = prim.attributes[k].data;
					if (normals->type != cgltf_type_vec3 || normals->component_type != cgltf_component_type_r_32f) {
						fprintf(stderr, "unsupported vertex attribute normal type\n");
						abort();
					}
				} else if (prim.attributes[k].type == cgltf_attribute_type_texcoord) {
					uvs = prim.attributes[k].data;
					if (
						uvs->type != cgltf_type_vec2 ||
						(
							uvs->component_type != cgltf_component_type_r_32f &&
							uvs->component_type != cgltf_component_type_r_8u &&
							uvs->component_type != cgltf_component_type_r_16u
						)
					) {
						fprintf(stderr, "unsupported vertex attribute texcoord type\n");
						abort();
					}
				}

				if (positions != NULL && normals != NULL && uvs != NULL) break;
			}

			if (normals == NULL || uvs == NULL) {
				fprintf(stderr, "normal and uv attributes are required\n");
				abort();
			}

			if (positions->count != normals->count || normals->count != uvs->count) {
				fprintf(stderr, "position, normal and uv attribute counts should be equal\n");
				abort();
			}

			cgltf_accessor_unpack_indices(prim.indices, indcs + indcs_i, 4, prim.indices->count);

			for (int k = 0; k < prim.indices->count; k++) {
				*(indcs + indcs_i + k) += (uint32_t)attrs_i;
			}

			mesh_o->indices_idx = indcs_i;
			mesh_o->indices_num = prim.indices->count;
			indcs_i += prim.indices->count;

			size_t norm_off = cgltf_accessor_unpack_floats(positions, attrs_buf, positions->count * 3);
			size_t uv_off = norm_off + cgltf_accessor_unpack_floats(normals, attrs_buf + norm_off, positions->count * 3);
			cgltf_accessor_unpack_floats(uvs, attrs_buf + uv_off, positions->count * 2);

			cgltf_float world_mat_raw[16];
			cgltf_node_transform_world(node, world_mat_raw);

			mat4s world_mat = glms_mat4_make(world_mat_raw);
			mat4s world_norm_mat = glms_mat4_transpose(glms_mat4_inv(world_mat));

			for (int k = 0; k < positions->count; k++) {
				struct attr_fmt *attr = &attrs[attrs_i];

				vec3s pos = glms_mat4_mulv3(world_mat, (vec3s){*(attrs_buf + (k * 3)), *(attrs_buf + (k * 3) + 1), *(attrs_buf + (k * 3) + 2)}, 0.0f);
				vec3s norm = glms_mat4_mulv3(world_norm_mat, (vec3s){*(attrs_buf + norm_off + (k * 3)), *(attrs_buf + norm_off + (k * 3) + 1), *(attrs_buf + norm_off + (k * 3) + 2)}, 0.0f);

				attr->position[0] = pos.x;
				attr->position[1] = pos.y;
				attr->position[2] = pos.z;

				attr->normal[0] = norm.x;
				attr->normal[1] = norm.y;
				attr->normal[2] = norm.z;

				attr->uv[0] = *(attrs_buf + uv_off + (k * 2));
				attr->uv[1] = *(attrs_buf + uv_off + (k * 2) + 1);

				attrs_i++;
			}

			if (prim.material != NULL && prim.material->has_pbr_metallic_roughness) {
				struct material_fmt *mat = &mats[mats_i];

				if (prim.material->pbr_metallic_roughness.base_color_texture.texture != NULL) {
					struct texture_fmt *tex = &texs[texs_i];
					model_process_load_image(prim.material->pbr_metallic_roughness.base_color_texture.texture->image, tex, dir, f);
					mat->albedo_map = texs_i;
					texs_i++;
				} else {
					mat->albedo_map = -1;
				}

				mat->albedo_tint[0] = prim.material->pbr_metallic_roughness.base_color_factor[0];
				mat->albedo_tint[1] = prim.material->pbr_metallic_roughness.base_color_factor[1];
				mat->albedo_tint[2] = prim.material->pbr_metallic_roughness.base_color_factor[2];
				mat->albedo_tint[3] = prim.material->pbr_metallic_roughness.base_color_factor[3];

				mesh_o->material_idx = mats_i;
				mats_i++;
			}

			meshes_i++;
		}
	}

	assert(meshes_num == meshes_i);
	assert(indcs_num == indcs_i);
	assert(attrs_num == attrs_i);
	assert(mats_num == mats_i);
	assert(texs_num == texs_i);

	header->meshes_off = ftell(f);
	header->meshes_num = meshes_num;
	fwrite(meshes, sizeof(struct mesh_fmt), meshes_num, f);

	header->indices_off = ftell(f);
	header->indices_num = indcs_num;
	fwrite(indcs, sizeof(uint32_t), indcs_num, f);

	header->attrs_off = ftell(f);
	header->attrs_num = attrs_num;
	fwrite(attrs, sizeof(struct attr_fmt), attrs_num, f);

	if (mats_num == 0) {
		header->materials_off = 0;
		header->materials_num = 0;
	} else {
		header->materials_off = ftell(f);
		header->materials_num = mats_num;
		fwrite(mats, sizeof(struct material_fmt), mats_num, f);
	}

	if (texs_num == 0) {
		header->textures_off = 0;
		header->textures_num = 0;
	} else {
		header->textures_off = ftell(f);
		header->textures_num = mats_num;
		fwrite(texs, sizeof(struct texture_fmt), texs_num, f);
	}

	free(attrs_buf);
	free(meshes);
	free(indcs);
	free(attrs);
	free(mats);
	free(texs);
}

static inline void model_process_load_image(cgltf_image *img, struct texture_fmt *tex, const char *dir, FILE *f) {
	if (img->uri != NULL) {
		if (strncmp(img->uri, "data:", 5) == 0) {
			size_t comma_i = 0;
			while(img->uri[comma_i] != ',' && img->uri[comma_i] != '\0') comma_i++;
			if (img->uri[comma_i] == '\0') {
				fprintf(stderr, "invalid base64 image\n");
				abort();
			}

			size_t b64_size = strlen(img->uri + comma_i + 1);
			size_t pad_size = 0;
			while(img->uri[comma_i + (b64_size - pad_size)]) pad_size++;

			size_t out_size = (3 * (b64_size / 4)) - (pad_size);
			void *out = NULL;

			cgltf_options opts = {0};
			cgltf_result res = cgltf_load_buffer_base64(&opts, out_size, img->uri + comma_i + 1, &out);
			if (res != cgltf_result_success) {
				fprintf(stderr, "failed to load base64 buffer\n");
				abort();
			}

			int w, h, n;
			stbi_uc *data = stbi_load_from_memory(out, out_size, &w, &h, &n, 4);
			if (data == NULL) {
				fprintf(stderr, "stbi error %s\n", stbi_failure_reason());
				abort();
			}

			tex->off = ftell(f);
			tex->len = w * h * 4 * sizeof(stbi_uc);
			tex->width = w;
			tex->height = h;

			fwrite(data, sizeof(stbi_uc), w * h * 4, f);

			stbi_image_free(data);
			free(out);
		} else {
			char path[strlen(dir) + strlen(img->uri) + 2];
			sprintf(path, "%s/%s", dir, img->uri);

			int w, h, n;
			stbi_uc *data = stbi_load(path, &w, &h, &n, 4);
			if (data == NULL) {
				fprintf(stderr, "stbi error %s\n", stbi_failure_reason());
				abort();
			}

			tex->off = ftell(f);
			tex->len = w * h * 4 * sizeof(stbi_uc);
			tex->width = w;
			tex->height = h;

			fwrite(data, sizeof(stbi_uc), w * h * 4, f);

			stbi_image_free(data);
		}
	} else if (img->buffer_view->buffer->data != NULL) {
		uint8_t *buf = mallocs(img->buffer_view->size);
		int offset = (int)img->buffer_view->offset;
		int stride = (int)img->buffer_view->stride ? (int)img->buffer_view->stride : 1;

		for (size_t i = 0; i < img->buffer_view->size; i++) {
			buf[i] = ((uint8_t*)img->buffer_view->buffer->data)[offset];
			offset += stride;
		}

		int w, h, n;
		stbi_uc *data = stbi_load_from_memory(buf, img->buffer_view->size, &w, &h, &n, 4);
		if (data == NULL) {
			fprintf(stderr, "stbi error %s\n", stbi_failure_reason());
			abort();
		}

		tex->off = ftell(f);
		tex->len = w * h * 4 * sizeof(stbi_uc);
		tex->width = w;
		tex->height = h;

		fwrite(data, sizeof(stbi_uc), w * h * 4, f);

		stbi_image_free(data);
		free(buf);
	}
}
