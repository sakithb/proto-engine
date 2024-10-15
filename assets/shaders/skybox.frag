#version 460 core

in vec3 frag_uv;

out vec4 frag_color;

uniform samplerCube skybox;

void main() {    
 	frag_color = texture(skybox, frag_uv);
	// frag_color = vec4((frag_uv + 1.0f) / 2.0f, 1.0f);
}
