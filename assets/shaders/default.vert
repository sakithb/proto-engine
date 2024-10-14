#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 frag_pos;
out vec3 frag_wpos;
out vec3 frag_normal;
out vec2 frag_uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
	frag_pos = pos;
	frag_wpos = (model * vec4(pos, 1.0)).xyz;
	frag_normal = normal;
	frag_uv = uv;
}
