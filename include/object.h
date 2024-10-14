#ifndef OBJECT_H
#define OBJECT_H

#include "glad/gl.h"
#include "cglm/struct.h"
#include "camera.h"
#include "model.h"

struct object {
	vec3s translation;
	vec3s rotation;
	vec3s scale;

	struct model model;
};

void object_init(struct object *o, const char *model);
void object_draw(struct object *o, GLuint shader);
void object_free(struct object *o);

#endif
