#include <string.h>
#include "shader.h"
#include "light.h"

void light_point_set_uniforms(struct point_light *light, int index, GLuint shader) {
	char attr[20] = "lights[0].";
	attr[7] = index + 48;

	strcpy(attr + 10, "position");
	shader_set_vec3(shader, attr, light->position.raw);

	strcpy(attr + 10, "ambient");
	shader_set_vec3(shader, attr, light->ambient.raw);

	strcpy(attr + 10, "diffuse");
	shader_set_vec3(shader, attr, light->diffuse.raw);

	strcpy(attr + 10, "specular");
	shader_set_vec3(shader, attr, light->specular.raw);

	strcpy(attr + 10, "constant");
	shader_set_float(shader, attr, light->constant);

	strcpy(attr + 10, "linear");
	shader_set_float(shader, attr, light->linear);

	strcpy(attr + 10, "quadratic");
	shader_set_float(shader, attr, light->quadratic);
}

void light_directional_set_uniforms(struct directional_light *light, GLuint shader) {
	shader_set_vec3(shader, "sun.direction", light->direction.raw);
	shader_set_vec3(shader, "sun.ambient", light->ambient.raw);
	shader_set_vec3(shader, "sun.diffuse", light->diffuse.raw);
	shader_set_vec3(shader, "sun.specular", light->specular.raw);
}
