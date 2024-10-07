#include <stdint.h>
#include <stddef.h>
#include "glad/gl.h"
#include "cgltf.h"
#include "stb_image.h"
#include "memutils.h"
#include "shader.h"
#include "model.h"

static inline void model_process_load_image(cgltf_image *img, GLuint texture, const char *dir);

struct material default_material = {NULL, (vec4s){0.5f, 0.5f, 0.5f, 1.0f}};

void model_init(struct model *model, const char *path) {
	char *dir_end = strrchr(path, '/');
	size_t dir_len;
	if (dir_end == NULL) {
		dir_len = 0;
	} else {
		dir_len = dir_end - path;
	}

	char dir[dir_len + 1];
	strncpy(dir, path, dir_len);
	dir[dir_len] = '\0';

	cgltf_options opts = {0};
	cgltf_data *data;
	cgltf_result result = cgltf_parse_file(&opts, path, &data);
	if (result != cgltf_result_success) {
		fprintf(stderr, "Failed to parse file");
		abort();
	}

	result = cgltf_load_buffers(&opts, data, path);
	if (result != cgltf_result_success) {
		fprintf(stderr, "Failed to load buffers");
		abort();
	}

	size_t meshes_num = 0, materials_num = 0, textures_num = 0;
	for (int i = 0; i < data->nodes_count; i++) {
		cgltf_node *node = data->nodes + i;
		cgltf_mesh *mesh = node->mesh;
		if (mesh == NULL) continue;

		for (int j = 0; j < mesh->primitives_count; j++) {
			cgltf_primitive *prim = mesh->primitives + j;

			assert(prim->type == cgltf_primitive_type_triangles);

			if (prim->material != NULL && prim->material->has_pbr_metallic_roughness) {
				if (prim->material->pbr_metallic_roughness.base_color_texture.texture != NULL) {
					textures_num++;
				}

				materials_num++;
			}

			meshes_num++;
		}
	}

	struct mesh *meshes = mallocs(meshes_num * sizeof(struct mesh));
	struct material *materials = mallocs(materials_num * sizeof(struct material));
	GLuint *textures = mallocs(textures_num * sizeof(GLuint));

	glGenTextures(textures_num, textures);

	size_t meshes_i = 0, materials_i = 0, textures_i = 0;

	for (int i = 0; i < data->nodes_count; i++) {
		cgltf_node *node = data->nodes + i;
		cgltf_mesh *mesh = node->mesh;
		if (mesh == NULL) continue;

		for (int j = 0; j < mesh->primitives_count; j++) {
			cgltf_primitive *prim = mesh->primitives + j;

			struct mesh *mesh_o = meshes + meshes_i;

			cgltf_accessor *positions = NULL, *uvs = NULL;

			for (int k = 0; k < prim->attributes_count; k++) {
				cgltf_attribute *attr = prim->attributes + k;
				if (attr->type == cgltf_attribute_type_position) {positions = attr->data;}
				else if (attr->type == cgltf_attribute_type_texcoord) {uvs = attr->data;}

				if (positions != NULL && uvs != NULL) break;
			}

			assert(positions != NULL);
			assert(uvs != NULL);

			size_t pos_num_floats = cgltf_accessor_unpack_floats(positions, NULL, 0);
			float *pos_buf = callocs(pos_num_floats, sizeof(float));
			cgltf_accessor_unpack_floats(positions, pos_buf, pos_num_floats);

			cgltf_float world_mat_raw[16] = {0};
			cgltf_node_transform_world(node, world_mat_raw);

			mat4 world_mat = {0};
			glm_mat4_make(world_mat_raw, world_mat);

			for (int k = 0; k < positions->count; k++) {
				vec4 v = {pos_buf[k*3], pos_buf[k*3+1], pos_buf[k*3+2], 1.0f};

				vec4 vr = {0};
				glm_mat4_mulv(world_mat, v, vr);

				pos_buf[k*3] = vr[0];
				pos_buf[k*3+1] = vr[1];
				pos_buf[k*3+2] = vr[2];
			}

			size_t uvs_num_floats = cgltf_accessor_unpack_floats(uvs, NULL, 0);
			float *uvs_buf = callocs(uvs_num_floats, sizeof(float));
			cgltf_accessor_unpack_floats(uvs, uvs_buf, uvs_num_floats);
	
			size_t idx_num = 0;
			GLuint *idx_buf;
			if (prim->indices != NULL) {
				idx_num = prim->indices->count;
				idx_buf = callocs(idx_num, sizeof(GLuint));
				cgltf_accessor_unpack_indices(prim->indices, idx_buf, 4, idx_num);
			} else {
				idx_num = positions->count;
				idx_buf = callocs(idx_num, sizeof(GLuint));
				for (int k = 0; k < idx_num; k++) {
					idx_buf[k] = (GLuint)k;
				}
			}
			
			GLuint vao, vbo_pos, vbo_uvs, ebo;
			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo_pos);
			glGenBuffers(1, &vbo_uvs);
			glGenBuffers(1, &ebo);

			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
			glBufferData(GL_ARRAY_BUFFER, pos_num_floats * sizeof(float), pos_buf, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
			glBufferData(GL_ARRAY_BUFFER, uvs_num_floats * sizeof(float), uvs_buf, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_num * sizeof(GLuint), idx_buf, GL_STATIC_DRAW);

			mesh_o->vao = vao;
			mesh_o->vbo_pos = vbo_pos;
			mesh_o->vbo_uvs = vbo_uvs;
			mesh_o->ebo = ebo;
			mesh_o->indices_num = idx_num;

			if (prim->material != NULL && prim->material->has_pbr_metallic_roughness) {
				struct material *material = materials + materials_i;

				if (prim->material->pbr_metallic_roughness.base_color_texture.texture != NULL) {
					material->albedo_map = textures + textures_i;
					model_process_load_image(prim->material->pbr_metallic_roughness.base_color_texture.texture->image, *material->albedo_map, dir);
					textures_i++;
				} else {
					material->albedo_map = NULL;
				}

				material->albedo_tint.r = prim->material->pbr_metallic_roughness.base_color_factor[0];
				material->albedo_tint.g = prim->material->pbr_metallic_roughness.base_color_factor[1];
				material->albedo_tint.b = prim->material->pbr_metallic_roughness.base_color_factor[2];
				material->albedo_tint.a = prim->material->pbr_metallic_roughness.base_color_factor[3];

				mesh_o->material = material;
				materials_i++;
			} else {
				mesh_o->material = &default_material;
			}

			free(pos_buf);
			free(uvs_buf);
			free(idx_buf);

			meshes_i++;
		}
	}

	model->meshes = meshes;
	model->meshes_num = meshes_num;
	model->materials = materials;
	model->materials_num = materials_num;
	model->textures = textures;
	model->textures_num = textures_num;

	cgltf_free(data);
}

void model_draw(struct model *model, GLuint shader) {
	for (int i = 0; i < model->meshes_num; i++) {
		struct mesh *mesh = model->meshes + i;

		if (mesh->material->albedo_map != NULL) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *mesh->material->albedo_map);
			shader_set_int(shader, "mat.albedo_map", 0);
			shader_set_int(shader, "mat.albedo_set", 1);
		} else {
			shader_set_int(shader, "mat.albedo_map", 0);
			shader_set_int(shader, "mat.albedo_set", 0);
		}

		shader_set_vec4(shader, "mat.albedo_tint", mesh->material->albedo_tint.raw);

		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indices_num, GL_UNSIGNED_INT, 0);
	}
}

void model_free(struct model *model) {
	for (int i = 0; i < model->meshes_num; i++) {
		struct mesh *mesh = model->meshes + i;
		glDeleteVertexArrays(1, &mesh->vao);
		glDeleteBuffers(1, &mesh->vbo_pos);
		glDeleteBuffers(1, &mesh->vbo_uvs);
		glDeleteBuffers(1, &mesh->ebo);
	}

	glDeleteTextures(model->textures_num, model->textures);

	free(model->meshes);
	free(model->materials);
	free(model->textures);
}

static inline void model_process_load_image(cgltf_image *img, GLuint texture, const char *dir) {
	int width, height, num_components;
	stbi_uc *data;

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

			data = stbi_load_from_memory(out, out_size, &width, &height, &num_components, 4);
			if (data == NULL) {
				fprintf(stderr, "stbi error %s\n", stbi_failure_reason());
				abort();
			}

			free(out);
		} else {
			char path[strlen(dir) + strlen(img->uri) + 2];
			sprintf(path, "%s/%s", dir, img->uri);

			data = stbi_load(path, &width, &height, &num_components, 4);
			if (data == NULL) {
				fprintf(stderr, "stbi error %s\n", stbi_failure_reason());
				abort();
			}
		}
	} else if (img->buffer_view->buffer->data != NULL) {
		uint8_t *buf = mallocs(img->buffer_view->size);
		int offset = (int)img->buffer_view->offset;
		int stride = (int)img->buffer_view->stride ? (int)img->buffer_view->stride : 1;

		for (size_t i = 0; i < img->buffer_view->size; i++) {
			buf[i] = ((uint8_t*)img->buffer_view->buffer->data)[offset];
			offset += stride;
		}

		data = stbi_load_from_memory(buf, img->buffer_view->size, &width, &height, &num_components, 4);
		if (data == NULL) {
			fprintf(stderr, "stbi error %s\n", stbi_failure_reason());
			abort();
		}

		free(buf);
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
}
