#ifndef LIGHT_H
#define LIGHT_H

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

#endif
