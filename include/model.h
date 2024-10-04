#ifndef MODEL_H
#define MODEL_H

#include <glad/gl.h>
#include <cglm/struct.h>

struct material {
	GLuint *diffuse_map;
	GLuint *specular_map;
};

struct mesh {
	struct material *material;
	GLsizei ebo_off;
	GLsizei ebo_num;
};

struct model {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

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
