#include <math.h>
#include "cglm/struct.h"
#include "global.h"
#include "shader.h"
#include "camera.h"

static void camera_update_axes(struct camera *cam);

void camera_init(struct camera *cam, vec3s pos) {
	cam->pos = pos;
	cam->front = (vec3s){0.0f, 0.0f, -1.0f};
	cam->up = (vec3s){0.0f, 1.0f, 0.0f};
	cam->yaw = -90.0f;
	cam->pitch = 0.0f;
	camera_update_axes(cam);
}

void camera_rotate(struct camera *cam, float x_off, float y_off) {
	x_off *= 0.1f;
	y_off *= 0.1f;

	cam->yaw += x_off;
	cam->pitch += y_off;

	if (cam->pitch > 89.0f) {
		cam->pitch = 89.0f;
	} else if (cam->pitch < -89.0f) {
		cam->pitch = -89.0f;
	}

	camera_update_axes(cam);
}

void camera_move(struct camera *cam, enum camera_direction dir) {
	float velocity = 10.0f * state.delta_time;

	if (dir == CAM_FORWARD) {
		cam->pos = glms_vec3_add(cam->pos, glms_vec3_scale(cam->front, velocity));
	}

	if (dir == CAM_BACKWARD) {
		cam->pos = glms_vec3_sub(cam->pos, glms_vec3_scale(cam->front, velocity));
	}

	if (dir == CAM_LEFT) {
		cam->pos = glms_vec3_sub(cam->pos, glms_vec3_scale(cam->right, velocity));
	}

	if (dir == CAM_RIGHT) {
		cam->pos = glms_vec3_add(cam->pos, glms_vec3_scale(cam->right, velocity));
	}
}

void camera_set_uniforms(struct camera *cam, GLuint shader) {
	mat4s view = camera_lookat(cam);
	mat4s projection = glms_perspective(45.0f, (float)state.scr_width/(float)state.scr_height, 0.1f, 1000.0f);
	shader_set_mat4(shader, "view", view.raw);
	shader_set_mat4(shader, "projection", projection.raw);
	shader_set_vec3(shader, "view_pos", cam->pos.raw);
}

mat4s camera_lookat(struct camera *cam) {
	return glms_lookat(cam->pos, glms_vec3_add(cam->pos, cam->front), cam->up);
}

static void camera_update_axes(struct camera *cam) {
	vec3s front;
	front.x = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
	front.y = sin(glm_rad(cam->pitch));
	front.z = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));

	cam->front = glms_normalize(front);
	cam->right = glms_normalize(glms_cross(cam->front, (vec3s){0.0f, 1.0f, 0.0f}));
	cam->up = glms_normalize(glms_cross(cam->right, cam->front));
}
