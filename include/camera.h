#ifndef CAMERA_H
#define CAMERA_H

#include "glad/gl.h"
#include "cglm/struct.h"

enum camera_direction {
	CAM_FORWARD,
	CAM_BACKWARD,
	CAM_LEFT,
	CAM_RIGHT
};

struct camera {
	vec3s pos;
	vec3s front;
	vec3s up;
	vec3s right;

	float yaw;
	float pitch;
};

void camera_init(struct camera *cam, vec3s pos);
void camera_move(struct camera *cam, enum camera_direction dir);
void camera_rotate(struct camera *cam, float x_off, float y_off);
void camera_set_uniforms(struct camera *cam, GLuint shader);
mat4s camera_lookat(struct camera *cam);

#endif
