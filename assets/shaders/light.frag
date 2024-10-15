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

in vec2 frag_uv;

out vec4 frag_color;

uniform pbr_material material;

void main() {    
	frag_color = vec4(1.0);
}
