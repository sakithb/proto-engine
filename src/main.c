#include <stdio.h>
#include <string.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "global.h"
#include "model_mgr.h"
#include "setup.h"
#include "input.h"
#include "shader.h"
#include "camera.h"
#include "light.h"
#include "skybox.h"
#include "object.h"

int main() {
	GLFWwindow *window = setup();

	camera_init(&state.camera, (vec3s){0.0f, 3.0f, 10.0f});

	GLuint obj_shader;
	shader_init(&obj_shader, "assets/shaders/default.vert", "assets/shaders/default.frag");

	GLuint sky_shader;
	shader_init(&sky_shader, "assets/shaders/skybox.vert", "assets/shaders/skybox.frag");

	struct object car;
	object_init(&car, "assets/models/hovercar.glb");
	car.translation.x = -10.0f;

	struct object man;
	object_init(&man, "assets/models/man.glb");
	man.translation.z = -10.0f;

	struct object sorceress;
	object_init(&sorceress, "assets/models/sorceress.glb");
	sorceress.translation.z = 10.0f;
	sorceress.rotation.y = glm_rad(180.0f);

	struct object ground;
	object_init(&ground, "assets/models/ground.glb");

	struct directional_light directional_light;
	directional_light.direction = (vec3s){-0.2f, -1.0f, -0.3f};
	directional_light.ambient = (vec3s){0.2f, 0.2f, 0.2f};
	directional_light.diffuse = (vec3s){0.8f, 0.8f, 0.8f};
	directional_light.specular = (vec3s){1.0f, 1.0f, 1.0f};

	struct skybox skybox;
	skybox_load(&skybox, (const char*[6]){
		"assets/skybox/px.jpg",
		"assets/skybox/nx.jpg",
		"assets/skybox/py.jpg",
		"assets/skybox/ny.jpg",
		"assets/skybox/pz.jpg",
		"assets/skybox/nz.jpg",
	});

	while(!glfwWindowShouldClose(window)) {
		state.delta_time = glfwGetTime() - state.last_frame_time;
		state.last_frame_time = glfwGetTime();

		process_input(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(sky_shader);

		camera_set_uniforms(&state.camera, sky_shader);

		glDepthFunc(GL_LEQUAL);
		skybox_draw(&skybox, sky_shader);
		glDepthFunc(GL_LESS);

		glUseProgram(obj_shader);

		camera_set_uniforms(&state.camera, obj_shader);

		light_directional_set_uniforms(&directional_light, obj_shader);

		shader_set_int(obj_shader, "lights_num", 0);

		object_draw(&car, obj_shader);
		object_draw(&man, obj_shader);
		object_draw(&sorceress, obj_shader);
		object_draw(&ground, obj_shader);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	skybox_free(&skybox);

	object_free(&car);
	object_free(&man);
	object_free(&sorceress);
	object_free(&ground);

	model_mgr_free(&state.model_mgr);

	glDeleteProgram(obj_shader);
	glDeleteProgram(sky_shader);

	glfwTerminate();

	return 0;
}
