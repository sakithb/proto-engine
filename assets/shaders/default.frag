#version 460 core

#define MAX_LIGHTS 16
#define AMBIENT_COLOR vec3(1.0)
#define AMBIENT_INTENSITY 0.05

struct point_light {
	vec3 pos;
	vec3 color;
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
uniform point_light lights[MAX_LIGHTS];
uniform int lights_num;

void main() {    
	point_light light = lights[0];

	// frag_color = vec4((sin(frag_uv) + 1.0f) / 2.0f, 1.0f, 1.0f);
	frag_color = texture(material.albedo_map, frag_uv) * material.albedo_factor;

	vec3 ambient = AMBIENT_INTENSITY * AMBIENT_COLOR;

	vec3 normal = normalize(mat3(frag_normal_mx) * frag_normal);
	vec3 light_dir = light.pos - frag_pos;
	vec3 diffuse = 0.2 * max(dot(normal, light_dir), 0.0) * light.color;

	frag_color = vec4((ambient + diffuse), 1.0) * frag_color;
}
