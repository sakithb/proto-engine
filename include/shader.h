#ifndef SHADER_H
#define SHADER_H

#include "glad/gl.h"
#include "cglm/cglm.h"

void shader_init(GLuint *s, const char *vert_path, const char *frag_path);

void shader_set_bool(GLuint s, const char *name, bool v);
void shader_set_int(GLuint s, const char *name, int v);
void shader_set_float(GLuint s, const char *name, float v);

void shader_set_vec2(GLuint s, const char *name, vec2 v);
void shader_set_2f(GLuint s, const char *name, float x, float y);

void shader_set_vec3(GLuint s, const char *name, vec3 v);
void shader_set_3f(GLuint s, const char *name, float x, float y, float z);

void shader_set_vec4(GLuint s, const char *name, vec4 v);
void shader_set_4f(GLuint s, const char *name, float x, float y, float z, float w);

void shader_set_mat2(GLuint s, const char *name, mat2 m);
void shader_set_mat3(GLuint s, const char *name, mat3 m);
void shader_set_mat4(GLuint s, const char *name, mat4 m);

#endif
