#version 460 core

struct material {
	sampler2D diffuse;
	sampler2D specular;
};

in vec2 uv;

out vec4 color;

uniform material mat;

void main() {    
    //color = vec4((sin(uv) + 1.0f) / 2.0f, 1.0f, 1.0f);
	color = texture(mat.diffuse, uv);
}
