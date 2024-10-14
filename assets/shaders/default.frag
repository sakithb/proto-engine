#version 460 core

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
in vec3 frag_wpos;
in vec3 frag_normal;
in vec2 frag_uv;

out vec4 frag_color;

uniform pbr_material material;

void main() {    
	// frag_color = vec4((sin(frag_uv) + 1.0f) / 2.0f, 1.0f, 1.0f);
	frag_color = texture(material.albedo_map, frag_uv) * material.albedo_factor;
}
