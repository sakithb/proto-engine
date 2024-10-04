#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "memutils.h"
#include "model_fmt.h"

void model_process(const char *path);
void model_process_meshes(const struct aiScene *scene, struct header_fmt *header, FILE *f);
void model_process_materials(const struct aiScene *scene, struct header_fmt *header, FILE *f);

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		model_process(argv[i]);
	}

	return 0;
}

void model_process(const char *path) {
	char *p = strrchr(path, '/');
	if (p == NULL) {
		p = (char*)path;
	} else {
		p += 1;
	}

	char *d = strrchr(path, '.');
	if (d == NULL) {
		perror("Invalid file extension");
		abort();
	}

	char filepath[d - p + 21];
	strncpy(filepath, "assets/models/", 14);
	strncpy(filepath + 14, p, d - p);
	strcpy(filepath + 14 + (d - p), ".model");

	FILE *f = fopen(filepath, "wb");
	if (f == NULL) {
		fprintf(stderr, "Error opening file %s", filepath);
		abort();
	}

	struct header_fmt header = {0};
	fwrite(&header, sizeof(struct header_fmt), 1, f);

	const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_EmbedTextures);

	model_process_meshes(scene, &header, f);
	model_process_materials(scene, &header, f);

	fseek(f, 0, SEEK_SET);
	fwrite(&header, sizeof(struct header_fmt), 1, f);

	fclose(f);
}

void model_process_meshes(const struct aiScene *scene, struct header_fmt *header, FILE *f) {
	size_t indices_num = 0, vertices_num = 0;
	for (int i = 0; i < scene->mNumMeshes; i++) {
		const struct aiMesh *ai_mesh = scene->mMeshes[i];
		for (int j = 0; j < ai_mesh->mNumFaces; j++) {
			const struct aiFace face = ai_mesh->mFaces[j];
			indices_num += face.mNumIndices;
		}

		vertices_num += ai_mesh->mNumVertices;
	}

	struct mesh_fmt *meshes = mallocs(scene->mNumMeshes * sizeof(struct mesh_fmt));
	uint32_t *indices = mallocs(indices_num * sizeof(uint32_t));
	size_t indices_c = 0;
	float *vertices = mallocs(vertices_num * 8 * sizeof(float));
	size_t vertices_c = 0;
	for (int i = 0; i < scene->mNumMeshes; i++) {
		const struct aiMesh *ai_mesh = scene->mMeshes[i];
		struct mesh_fmt *mesh = &meshes[i];

		mesh->material_idx = ai_mesh->mMaterialIndex;
		mesh->indices_idx = indices_c;

		for (int j = 0; j < ai_mesh->mNumFaces; j++) {
			const struct aiFace face = ai_mesh->mFaces[j];
			for (int k = 0; k < face.mNumIndices; k++) {
				*(indices + indices_c) = face.mIndices[k];
				indices_c++;
			}
		}

		mesh->indices_num = indices_c - mesh->indices_idx;

		for (int j = 0; j < ai_mesh->mNumVertices; j++) {
			struct aiVector3D vertex = ai_mesh->mVertices[j];
			struct aiVector3D normal = ai_mesh->mNormals[j];
			struct aiVector3D uv = ai_mesh->mTextureCoords[0][j];

			float *p = vertices + (vertices_c * 8);
			p[0] = vertex.x;
			p[1] = vertex.y;
			p[2] = vertex.z;
			p[3] = normal.x;
			p[4] = normal.y;
			p[5] = normal.z;
			p[6] = uv.x;
			p[7] = uv.y;

			vertices_c++;
		}
	}

	header->meshes_off = ftell(f);
	header->meshes_num = scene->mNumMeshes;
	fwrite(meshes, sizeof(struct mesh_fmt), scene->mNumMeshes, f);

	header->indices_off = ftell(f);
	header->indices_num = indices_num;
	fwrite(indices, sizeof(uint32_t), indices_num, f);

	header->vertices_off = ftell(f);
	header->vertices_num = vertices_num;
	fwrite(vertices, sizeof(float) * 8, vertices_num, f);

	free(meshes);
	free(indices);
	free(vertices);
}

void model_process_materials(const struct aiScene *scene, struct header_fmt *header, FILE *f) {
	struct material_fmt *mats = mallocs(scene->mNumMaterials * sizeof(struct material_fmt));
	struct texture_fmt *texs = mallocs(scene->mNumTextures * sizeof(struct texture_fmt));
	size_t tex_c = 0;
	for (int i = 0; i < scene->mNumMaterials; i++) {
		const struct aiMaterial *ai_mat = scene->mMaterials[i];
		struct material_fmt *mat = &mats[i];
		struct aiString path;

		if (aiGetMaterialTextureCount(ai_mat, aiTextureType_DIFFUSE) > 0) {
			aiGetMaterialTexture(ai_mat, aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL);

			const struct aiTexture *ai_tex = aiGetEmbeddedTexture(scene, path.data);
			if (ai_tex != NULL) {
				if (ai_tex->mHeight == 0) {
					perror("Compressed textures not supported");
					abort();
				}

				size_t len = ai_tex->mWidth * ai_tex->mHeight * 4;

				struct texture_fmt *tex = &texs[tex_c];
				tex->off = ftell(f);
				tex->len = len;
				tex->width = ai_tex->mWidth;
				tex->height = ai_tex->mHeight;
				mat->diffuse_map_idx = tex_c;
				tex_c++;

				fwrite(ai_tex->pcData, sizeof(uint8_t), len, f);
			}
		}

		if (aiGetMaterialTextureCount(ai_mat, aiTextureType_SPECULAR) > 0) {
			aiGetMaterialTexture(ai_mat, aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL);

			const struct aiTexture *ai_tex = aiGetEmbeddedTexture(scene, path.data);
			if (ai_tex != NULL) {
				if (ai_tex->mHeight == 0) {
					perror("Compressed textures not supported");
					abort();
				}

				size_t len = ai_tex->mWidth * ai_tex->mHeight * 4;

				struct texture_fmt *tex = &texs[tex_c];
				tex->len = ftell(f);
				tex->off = len;
				tex->width = ai_tex->mWidth;
				tex->height = ai_tex->mHeight;
				mat->specular_map_idx = tex_c;
				tex_c++;

				fwrite(ai_tex->pcData, sizeof(uint8_t), len, f);
			}
		}
	}

	header->textures_off = ftell(f);
	header->textures_num = tex_c + 1;
	fwrite(texs, sizeof(struct texture_fmt), tex_c + 1, f);

	header->materials_off = ftell(f);
	header->materials_num = scene->mNumMaterials;
	fwrite(mats, sizeof(struct material_fmt), scene->mNumMaterials, f);

	free(texs);
	free(mats);
}
