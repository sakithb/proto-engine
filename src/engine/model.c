#include <stdint.h>
#include "glad/gl.h"
#include "memutils.h"
#include "shader.h"
#include "model_fmt.h"
#include "model.h"

void model_init(struct model *model, const char *path) {
	FILE *f = fopen(path, "rb");

	struct header_fmt header;
	fread(&header, sizeof(struct header_fmt), 1, f);

	glGenVertexArrays(1, &model->vao);
	glGenBuffers(1, &model->vbo);
	glGenBuffers(1, &model->ebo);

	glBindVertexArray(model->vao);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);

	int uint_diff = sizeof(GLuint) - sizeof(uint32_t);
	if (uint_diff < 0) {
		fprintf(stderr, "sizeof(GLuint) is smaller than sizeof(uint32_t)");
		abort();
	}
	size_t indices_size = header.indices_num * (uint_diff > 0 ? sizeof(GLuint) : sizeof(uint32_t));
	uint32_t *indices = mallocs(indices_size);
	fseek(f, header.indices_off, SEEK_SET);
	fread(indices, sizeof(uint32_t), header.indices_num, f);
	if (uint_diff > 0) {
		for (int i = header.indices_num - 1; i >= 0; i--) {
			uint32_t idx = indices[i];
			*(indices + (indices_size - ((header.indices_num - i) * sizeof(GLuint)))) = (GLuint)idx;
		}
	}
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
		fseek(f, header.textures_off + (i * sizeof(struct texture_fmt)), SEEK_SET);
		fread(&tex_fmt, sizeof(struct texture_fmt), 1, f);

		if (buf_size < tex_fmt.len) {
			buf_size = tex_fmt.len;
			buf = reallocs(buf, buf_size);
		}

		fseek(f, tex_fmt.off, SEEK_SET);
		fread(buf, 1, tex_fmt.len, f);

		GLuint tex = model->textures[i];
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_fmt.width, tex_fmt.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	fseek(f, header.materials_off, SEEK_SET);
	for (int i = 0; i < model->materials_num; i++) {
		struct material_fmt mat_fmt;
		fread(&mat_fmt, sizeof(struct material_fmt), 1, f);

		struct material *mat = &model->materials[i];

		if (mat_fmt.diffuse_map_idx != -1) {
			mat->diffuse_map = &model->textures[mat_fmt.diffuse_map_idx];
		} else {
			mat->diffuse_map = NULL;
		}

		if (mat_fmt.specular_map_idx != -1) {
			mat->specular_map = &model->textures[mat_fmt.specular_map_idx];
		} else {
			mat->specular_map = NULL;
		}
	}

	free(buf);
	fclose(f);
}

void model_draw(struct model *model, GLuint shader) {
	glBindVertexArray(model->vao);

	for (int i = 0; i < model->meshes_num; i++) {
		struct mesh *mesh = &model->meshes[i];

		if (mesh->material->diffuse_map != NULL) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *mesh->material->diffuse_map);
			shader_set_int(shader, "mat.diffuse", 0);
		} else {
			shader_set_int(shader, "mat.diffuse", -1);
		}


		if (mesh->material->specular_map != NULL) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, *mesh->material->specular_map);
			shader_set_int(shader, "mat.specular", 1);
		} else {
			shader_set_int(shader, "mat.specular", -1);
		}

		glDrawElements(GL_TRIANGLES, mesh->ebo_num, GL_UNSIGNED_INT, (void*)(uintptr_t)mesh->ebo_off);
	}
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
