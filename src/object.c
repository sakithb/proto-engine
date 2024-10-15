#include "cglm/struct.h"
#include "global.h"
#include "shader.h"
#include "model.h"
#include "model_mgr.h"
#include "object.h"

void object_init(struct object *o, const char *model) {
	o->translation = GLMS_VEC3_ZERO;
	o->rotation = GLMS_VEC3_ZERO;
	o->scale = GLMS_VEC3_ONE;

	o->model = model_mgr_get(&state.model_mgr, model);
}

void object_draw(struct object *o, GLuint shader) {
	mat4s model = glms_scale(glms_mul(glms_translate(GLMS_MAT4_IDENTITY, o->translation), glms_euler_xyz(o->rotation)), o->scale);
	shader_set_mat4(shader, "model", model.raw);
	model_draw(o->model, shader);
}

void object_free(struct object *o) {
	//model_free(o->model);
}
