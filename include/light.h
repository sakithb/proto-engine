#ifndef LIGHT_H
#define LIGHT_H

#include "glad/gl.h"
#include "cglm/struct.h"

struct point_light {
	vec3s position;  

	vec3s ambient;
	vec3s diffuse;
	vec3s specular;

	float constant;
	float linear;
	float quadratic;
}; 

struct directional_light {
	vec3s direction;

	vec3s ambient;
	vec3s diffuse;
	vec3s specular;
};

void light_point_set_uniforms(struct point_light *light, int index, GLuint shader);
void light_directional_set_uniforms(struct directional_light *light, GLuint shader);

#endif
