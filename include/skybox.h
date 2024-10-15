#ifndef SKYBOX_H
#define SKYBOX_H

#include "glad/gl.h"

struct skybox {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	GLuint tex;
};

void skybox_load(struct skybox *box, const char *faces[6]);
void skybox_draw(struct skybox *box, GLuint shader);
void skybox_free(struct skybox *box);

#endif
