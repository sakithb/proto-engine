#version 460 core

#define MAX_LIGHTS 16

struct point_light {
	vec3 position;  

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
}; 

struct directional_light {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct pbr_material {
	sampler2D albedo_map;
	sampler2D normal_map;
	sampler2D mr_map;
	sampler2D ao_map;

	vec4 albedo_factor;
	float metallic_factor;
	float roughness_factor;
};

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in mat4 frag_normal_mx;

out vec4 frag_color;

uniform pbr_material material;
uniform vec3 view_pos;

uniform point_light point_lights[MAX_LIGHTS];
uniform int point_lights_num;

uniform directional_light sun;

void main() {    
	frag_color = texture(material.albedo_map, frag_uv) * material.albedo_factor;

	float specular_fct = max(1 - (texture(material.mr_map, frag_uv).g * material.roughness_factor), 0.1);

	vec3 normal = normalize(mat3(frag_normal_mx) * frag_normal);
	vec3 view_dir = normalize(view_pos - frag_pos);

	vec3 diffuse_agg = vec3(0.0);
	vec3 ambient_agg = vec3(0.0);
	vec3 specular_agg = vec3(0.0);

	// Sun

	vec3 sun_dir = normalize(-sun.direction);
	vec3 sun_reflect_dir = reflect(-sun_dir, normal);  

	vec3 ambient = sun.ambient * frag_color.rgb;
	vec3 diffuse = max(dot(normal, sun_dir), 0.0) * sun.diffuse;
	vec3 specular = specular_fct * sun.specular * pow(max(dot(view_dir, sun_reflect_dir), 0.0), 32);

	ambient_agg += ambient;
	diffuse_agg += diffuse;
	specular_agg += specular;

	// Point lights

	for (int i = 0; i < point_lights_num; i++) {
		point_light light = point_lights[i];

		vec3 light_dir = light.position - frag_pos;
		vec3 reflect_dir = reflect(-light_dir, normal);  

		vec3 ambient = light.ambient * frag_color.rgb;
		vec3 diffuse = max(dot(normal, normalize(light_dir)), 0.0) * light.diffuse;
		vec3 specular = specular_fct * light.specular * pow(max(dot(view_dir, reflect_dir), 0.0), 32);

		float distance = length(light_dir);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		ambient_agg += ambient;
		diffuse_agg += diffuse;
	}

	frag_color = vec4((ambient_agg + diffuse_agg + specular_agg), 1.0) * frag_color;
}
