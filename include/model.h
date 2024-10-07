#ifndef MODEL_H
#define MODEL_H

#include <glad/gl.h>
#include <cglm/struct.h>

struct material {
	GLuint *albedo_map;
	vec4s albedo_tint;
};

struct mesh {
	GLuint vao;
	GLuint vbo_pos;
	GLuint vbo_uvs;
	GLuint ebo;

	struct material *material;
	size_t indices_num;
};

struct model {
	struct mesh *meshes;
	size_t meshes_num;

	struct material *materials;
	size_t materials_num;

	GLuint *textures;
	size_t textures_num;
};

void model_init(struct model *model, const char *path);
void model_draw(struct model *model, GLuint shader);
void model_free(struct model *model);

#endif
