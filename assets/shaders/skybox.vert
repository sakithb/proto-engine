#version 460 core

layout (location = 0) in vec3 pos;

out vec3 frag_uv;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = (projection * mat4(mat3(view)) * vec4(pos, 1.0)).xyww;
	frag_uv = pos;
}
