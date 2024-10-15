#include <stdio.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "global.h"
#include "setup.h"
#include "input.h"
#include "shader.h"
#include "camera.h"
#include "light.h"
#include "object.h"

int main() {
	GLFWwindow *window = setup();

	camera_init(&state.camera, (vec3s){0.0f, 3.0f, 10.0f});

	GLuint obj_shader;
	shader_init(&obj_shader, "assets/shaders/default.vert", "assets/shaders/default.frag");

	GLuint light_shader;
	shader_init(&light_shader, "assets/shaders/light.vert", "assets/shaders/light.frag");

	struct object man;
	object_init(&man, "assets/models/man.glb");
	man.translation.z = -5.0f;

	struct object sorceress;
	object_init(&sorceress, "assets/models/sorceress.glb");
	sorceress.translation.z = 5.0f;
	sorceress.rotation.y = glm_rad(180.0f);

	struct point_light light = {(vec3s){0.0f, 0.0f, 0.0f}, GLMS_VEC3_ONE, GLMS_VEC3_ZERO, GLMS_VEC3_ZERO, 0.0f, 0.0f, 0.0f};

	struct object light_obj;
	object_init(&light_obj, "assets/models/cubes.glb");
	light_obj.translation.y = light.position.y;
	light_obj.scale = glms_vec3_scale(light_obj.scale, 0.25);

	while(!glfwWindowShouldClose(window)) {
		state.delta_time = glfwGetTime() - state.last_frame_time;
		state.last_frame_time = glfwGetTime();

		process_input(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(light_shader);

		camera_update(&state.camera, (float)state.scr_width / state.scr_height, light_shader);
		object_draw(&light_obj, light_shader);

		glUseProgram(obj_shader);

		camera_update(&state.camera, (float)state.scr_width / state.scr_height, obj_shader);
		shader_set_vec3(obj_shader, "view_pos", state.camera.pos.raw);

		shader_set_vec3(obj_shader, "lights[0].pos", light.position.raw);
		shader_set_vec3(obj_shader, "lights[0].color", light.ambient.raw);
		shader_set_int(obj_shader, "lights_num", 1);

		object_draw(&man, obj_shader);
		object_draw(&sorceress, obj_shader);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	object_free(&man);
	object_free(&light_obj);
	object_free(&sorceress);

	glDeleteProgram(obj_shader);
	glDeleteProgram(light_shader);

	glfwTerminate();

	return 0;
}
