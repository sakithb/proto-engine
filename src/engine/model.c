#include <stdint.h>
#include "glad/gl.h"
#include "memutils.h"
#include "model_fmt.h"
#include "model.h"

void model_load(struct model *model, const char *path) {
	FILE *f = fopen(path, "rb");

	struct header_fmt header;
	fread(&header, sizeof(struct header_fmt), 1, f);

	glGenVertexArrays(1, &model->vao);
	glGenBuffers(1, &model->vbo);
	glGenBuffers(1, &model->ebo);

	glBindVertexArray(model->vao);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);

	size_t indices_size = header.indices_num * sizeof(uint32_t);
	uint32_t *indices = mallocs(indices_size);
	fseek(f, header.indices_off, SEEK_SET);
	fread(indices, sizeof(uint32_t), header.indices_num, f);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);

	size_t vertices_size = header.vertices_num * 8 * sizeof(float);
	float *vertices;
	if (vertices_size > indices_size) {
		vertices = reallocs(indices, vertices_size);
	} else {
		vertices = (float*)indices;
	}
	fseek(f, header.vertices_off, SEEK_SET);
	fread(vertices, 8 * sizeof(float), header.vertices_num, f);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	model->meshes = mallocs(header.meshes_num * sizeof(struct mesh));
	model->meshes_num = header.meshes_num;
	
	model->materials = mallocs(header.materials_num * sizeof(struct material));
	model->materials_num = header.materials_num;

	model->textures = mallocs(header.textures_num * sizeof(GLuint));
	model->textures_num = header.textures_num;
	glGenTextures(model->textures_num, model->textures);

	fseek(f, header.meshes_off, SEEK_SET);
	for (int i = 0; i < model->meshes_num; i++) {
		struct mesh_fmt mesh_fmt;
		fread(&mesh_fmt, sizeof(struct mesh_fmt), 1, f);

		struct mesh *mesh = &model->meshes[i];
		mesh->material = &model->materials[mesh_fmt.material_idx];
		mesh->ebo_off = mesh_fmt.indices_idx;
		mesh->ebo_num = mesh_fmt.indices_num;
	}

	size_t buf_size = vertices_size;
	uint8_t *buf = (uint8_t*)vertices;
	for (int i = 0; i < model->textures_num; i++) {
		struct texture_fmt tex_fmt;
		fseek(f, header.textures_off, SEEK_SET);
		fread(&tex_fmt, sizeof(struct texture_fmt), 1, f);

		if (buf_size > tex_fmt.len * sizeof(uint8_t)) {
			buf_size = tex_fmt.len * sizeof(uint8_t);
			buf = realloc(buf, buf_size);
		}

		fseek(f, tex_fmt.off, SEEK_SET);
		fread(buf, sizeof(uint8_t), tex_fmt.len, f);

		GLuint tex = model->textures[i];
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_fmt.width, tex_fmt.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	}


	fseek(f, header.materials_off, SEEK_SET);
	for (int i = 0; i < model->materials_num; i++) {
		struct material_fmt mat_fmt;
		fread(&mat_fmt, sizeof(struct material_fmt), 1, f);

		struct material *mat = &model->materials[i];
		mat->diffuse_map = model->textures[mat_fmt.diffuse_map_idx];
		mat->specular_map = model->textures[mat_fmt.specular_map_idx];
	}

	free(buf);
	fclose(f);
}

void model_free(struct model *model) {
	free(model->meshes);
	free(model->materials);
	free(model->textures);

	glDeleteBuffers(1, &model->vbo);
	glDeleteBuffers(1, &model->ebo);
	glDeleteTextures(model->textures_num, model->textures);
	glDeleteVertexArrays(1, &model->vao);
}
