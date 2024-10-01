#ifndef MODEL_H
#define MODEL_H

#include <glad/gl.h>
#include <cglm/struct.h>

struct material {
	GLuint diffuse_map;
	GLuint specular_map;
	GLfloat shininess;
};

struct mesh {
	struct material *material;
	GLsizei ebo_offset;
};

struct model {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	struct mesh *meshes;
	size_t meshes_num;

	GLuint *textures;
	size_t textures_num;
};

#endif
