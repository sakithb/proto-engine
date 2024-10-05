#include <stdint.h>
#include <stddef.h>
#include "glad/gl.h"
#include "memutils.h"
#include "shader.h"
#include "model_fmt.h"
#include "model.h"

struct material default_material = (struct material){.albedo_map = NULL, .albedo_tint = (vec4s){0.5f, 0.5f, 0.5f, 1.0f}};

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

	size_t attrs_size = header.attrs_num * sizeof(struct attr_fmt);
	struct attr_fmt *attrs;
	if (attrs_size > indices_size) {
		attrs = reallocs(indices, attrs_size);
	} else {
		attrs = (struct attr_fmt*)indices;
	}
	fseek(f, header.attrs_off, SEEK_SET);
	fread(attrs, sizeof(struct attr_fmt), header.attrs_num, f);
	glBufferData(GL_ARRAY_BUFFER, attrs_size, attrs, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct attr_fmt), (void*)offsetof(struct attr_fmt, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct attr_fmt), (void*)offsetof(struct attr_fmt, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct attr_fmt), (void*)offsetof(struct attr_fmt, uv));
	glEnableVertexAttribArray(2);

	model->meshes = mallocs(header.meshes_num * sizeof(struct mesh));
	model->meshes_num = header.meshes_num;
	
	if (header.materials_num == 0) {
		model->materials = NULL;
		model->materials_num = 0;
	} else {
		model->materials = mallocs(header.materials_num * sizeof(struct material));
		model->materials_num = header.materials_num;
	}

	if (header.textures_num == 0) {
		model->textures = NULL;
		model->textures_num = 0;
	} else {
		model->textures = mallocs(header.textures_num * sizeof(GLuint));
		model->textures_num = header.textures_num;
		glGenTextures(model->textures_num, model->textures);
	}

	fseek(f, header.meshes_off, SEEK_SET);
	for (int i = 0; i < model->meshes_num; i++) {
		struct mesh_fmt mesh_fmt;
		fread(&mesh_fmt, sizeof(struct mesh_fmt), 1, f);

		struct mesh *mesh = &model->meshes[i];

		if (model->materials == NULL || mesh_fmt.material_idx == -1) {
			mesh->material = &default_material;
		} else {
			mesh->material = &model->materials[mesh_fmt.material_idx];
		}

		mesh->ebo_off = mesh_fmt.indices_idx;
		mesh->ebo_num = mesh_fmt.indices_num;
	}

	size_t buf_size = attrs_size;
	uint8_t *buf = (uint8_t*)attrs;
	if (model->textures != NULL) {
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
	}

	if (model->materials != NULL) {
		fseek(f, header.materials_off, SEEK_SET);
		for (int i = 0; i < model->materials_num; i++) {
			struct material_fmt mat_fmt;
			fread(&mat_fmt, sizeof(struct material_fmt), 1, f);

			struct material *mat = &model->materials[i];

			if (mat_fmt.albedo_map != -1) {
				mat->albedo_map = &model->textures[mat_fmt.albedo_map];
			} else {
				mat->albedo_map = NULL;
			}

			mat->albedo_tint.r = mat_fmt.albedo_tint[0];
			mat->albedo_tint.g = mat_fmt.albedo_tint[1];
			mat->albedo_tint.b = mat_fmt.albedo_tint[2];
			mat->albedo_tint.a = mat_fmt.albedo_tint[3];
		}
	}

	free(buf);
	fclose(f);
}

void model_draw(struct model *model, GLuint shader) {
	glBindVertexArray(model->vao);

	for (int i = 0; i < model->meshes_num; i++) {
		struct mesh *mesh = &model->meshes[i];

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
