#include <stdio.h>
#include <stdlib.h>
#include "shader.h"
#include "memutils.h"

void shader_init(GLuint *s, const char *vert_path, const char *frag_path) {
	char *vert_code, *frag_code;
	FILE *vert_f, *frag_f;
	int vert_len, frag_len;
	GLuint vert_sh, frag_sh, prog;

	GLint success;
	GLchar log[1024];

	vert_f = fopen(vert_path, "r");
	fseek(vert_f, 0, SEEK_END);
	vert_len = ftell(vert_f);
	fseek(vert_f, 0, SEEK_SET);

	vert_code = mallocs(vert_len + 1);

	fread(vert_code, 1, vert_len, vert_f);
	fclose(vert_f);
	vert_code[vert_len] = 0;

	vert_sh = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_sh, 1, (const char *const *)&vert_code, NULL);
	glCompileShader(vert_sh);

	glGetShaderiv(vert_sh, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(vert_sh, 1024, NULL, log);
		fprintf(stderr, "Failed to compile vertex shader: %s", log);
		abort();
	}

	free(vert_code);

	frag_f = fopen(frag_path, "r");
	fseek(frag_f, 0, SEEK_END);
	frag_len = ftell(frag_f);
	fseek(frag_f, 0, SEEK_SET);

	frag_code = mallocs(frag_len + 1);

	fread(frag_code, 1, frag_len, frag_f);
	fclose(frag_f);
	frag_code[frag_len] = 0;

	frag_sh = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_sh, 1, (const char *const *)&frag_code, NULL);
	glCompileShader(frag_sh);

	glGetShaderiv(frag_sh, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(frag_sh, 1024, NULL, log);
		fprintf(stderr, "Failed to compile fragment shader: %s", log);
		abort();
	}

	free(frag_code);

	prog = glCreateProgram();
	glAttachShader(prog, vert_sh);
	glAttachShader(prog, frag_sh);
	glLinkProgram(prog);

	glGetShaderiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(frag_sh, 1024, NULL, log);
		fprintf(stderr, "Failed to link shader program: %s", log);
		abort();
	}

	glDeleteShader(vert_sh);
	glDeleteShader(frag_sh);

	*s = prog;
}

void shader_set_bool(GLuint s, const char* name, bool v) {
	glUniform1i(glGetUniformLocation(s, name), v);
}

void shader_set_int(GLuint s, const char* name, int v) {
	glUniform1i(glGetUniformLocation(s, name), v);
}

void shader_set_float(GLuint s, const char* name, float v) {
	glUniform1f(glGetUniformLocation(s, name), v);
}

void shader_set_vec2(GLuint s, const char* name, vec2 v) {
	glUniform2fv(glGetUniformLocation(s, name), 1, v);
}

void shader_set_2f(GLuint s, const char* name, float x, float y) {
	glUniform2f(glGetUniformLocation(s, name), x, y);
}

void shader_set_vec3(GLuint s, const char* name, vec3 v) {
	glUniform3fv(glGetUniformLocation(s, name), 1, v);
}

void shader_set_3f(GLuint s, const char* name, float x, float y, float z) {
	glUniform3f(glGetUniformLocation(s, name), x, y, z);
}

void shader_set_vec4(GLuint s, const char* name, vec4 v) {
	glUniform4fv(glGetUniformLocation(s, name), 1, v);
}

void shader_set_4f(GLuint s, const char* name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(s, name), x, y, z, w);
}

void shader_set_mat2(GLuint s, const char* name, mat2 m) {
	glUniformMatrix2fv(glGetUniformLocation(s, name), 1, GL_FALSE, (float*)m);
}

void shader_set_mat3(GLuint s, const char* name, mat3 m) {
	glUniformMatrix3fv(glGetUniformLocation(s, name), 1, GL_FALSE, (float*)m);
}

void shader_set_mat4(GLuint s, const char* name, mat4 m) {
	glUniformMatrix4fv(glGetUniformLocation(s, name), 1, GL_FALSE, (float*)m);
}
