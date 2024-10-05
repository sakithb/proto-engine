#version 460 core

struct material {
	sampler2D albedo_map;
	int albedo_set;
	vec4 albedo_tint;
};

in vec2 uv;

out vec4 color;

uniform material mat;

void main() {    
    //color = vec4((sin(uv) + 1.0f) / 2.0f, 1.0f, 1.0f);

	if (mat.albedo_set == 1) {
		color = texture(mat.albedo_map, uv) * mat.albedo_tint;
	} else {
		color = mat.albedo_tint;
	}
}
