#include <stdio.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "global.h"
#include "setup.h"
#include "input.h"
#include "shader.h"
#include "camera.h"
#include "object.h"

int main() {
	GLFWwindow *window = setup();

	camera_init(&state.camera, (vec3s){0.0f, 3.0f, 10.0f});

	GLuint shader;
	shader_init(&shader, "assets/shaders/default.vert", "assets/shaders/default.frag");

	struct object man;
	object_init(&man, "assets/models/man.glb");
	man.translation.z = -5.0f;

	struct object sorceress;
	object_init(&sorceress, "assets/models/sorceress.glb");
	sorceress.translation.z = 5.0f;
	sorceress.rotation.y = glm_rad(180.0f);

	while(!glfwWindowShouldClose(window)) {
		state.delta_time = glfwGetTime() - state.last_frame_time;
		state.last_frame_time = glfwGetTime();

		process_input(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		camera_update(&state.camera, (float)state.scr_width / state.scr_height, shader);

		object_draw(&man, shader);
		object_draw(&sorceress, shader);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	object_free(&man);
	object_free(&sorceress);

	glDeleteProgram(shader);

	glfwTerminate();

	return 0;
}
