#include <stdio.h>
#include <string.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "global.h"
#include "setup.h"
#include "input.h"
#include "shader.h"
#include "camera.h"
#include "light.h"
#include "object.h"

#define LIGHTS_NUM 0

int main() {
	GLFWwindow *window = setup();

	camera_init(&state.camera, (vec3s){0.0f, 3.0f, 10.0f});

	GLuint obj_shader;
	shader_init(&obj_shader, "assets/shaders/default.vert", "assets/shaders/default.frag");

	GLuint light_shader;
	shader_init(&light_shader, "assets/shaders/light.vert", "assets/shaders/light.frag");

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

	struct point_light point_lights[LIGHTS_NUM];
	struct object point_light_objs[LIGHTS_NUM];

	struct directional_light directional_light;
	directional_light.direction = (vec3s){-0.2f, -1.0f, -0.3f};
	directional_light.ambient = (vec3s){0.2f, 0.2f, 0.2f};
	directional_light.diffuse = (vec3s){0.8f, 0.8f, 0.8f};
	directional_light.specular = (vec3s){1.0f, 1.0f, 1.0f};

	for (int i = 0; i < LIGHTS_NUM; i++) {
		point_lights[i] = (struct point_light){(vec3s){0.0f, 100.0f, 0.0f}, (vec3s){0.2f, 0.2f, 0.2f}, (vec3s){0.5f, 0.5f, 0.5f}, GLMS_VEC3_ONE, 1.0f, 0.09, 0.032f};

		object_init(point_light_objs + i, "assets/models/cubes.glb");
		point_light_objs[i].translation.x = point_lights[i].position.x;
		point_light_objs[i].translation.y = point_lights[i].position.y;
		point_light_objs[i].translation.z = point_lights[i].position.z;
		point_light_objs[i].scale = glms_vec3_scale(point_light_objs[i].scale, 0.25);
	}

	while(!glfwWindowShouldClose(window)) {
		state.delta_time = glfwGetTime() - state.last_frame_time;
		state.last_frame_time = glfwGetTime();

		process_input(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// glUseProgram(light_shader);

		// camera_update(&state.camera, (float)state.scr_width / state.scr_height, light_shader);


		// for (int i = 0; i < LIGHTS_NUM; i++) {
		// 	//point_lights[i].position.x = state.camera.pos.x;
		// 	//point_lights[i].position.y = state.camera.pos.y;
		// 	//point_lights[i].position.z = state.camera.pos.z + 1.0f;
		// 	object_draw(&point_light_objs[i], light_shader);
		// }

		glUseProgram(obj_shader);

		camera_update(&state.camera, (float)state.scr_width / state.scr_height, obj_shader);
		shader_set_vec3(obj_shader, "view_pos", state.camera.pos.raw);

		shader_set_vec3(obj_shader, "sun.direction", directional_light.direction.raw);
		shader_set_vec3(obj_shader, "sun.ambient", directional_light.ambient.raw);
		shader_set_vec3(obj_shader, "sun.diffuse", directional_light.diffuse.raw);
		shader_set_vec3(obj_shader, "sun.specular", directional_light.specular.raw);

		for (int i = 0; i < LIGHTS_NUM; i++) {
			char attr[20] = "lights[0].";
			attr[7] = i + 48;

			strcpy(attr + 10, "position");
			shader_set_vec3(obj_shader, attr, point_lights[i].position.raw);

			strcpy(attr + 10, "ambient");
			shader_set_vec3(obj_shader, attr, point_lights[i].ambient.raw);

			strcpy(attr + 10, "diffuse");
			shader_set_vec3(obj_shader, attr, point_lights[i].diffuse.raw);

			strcpy(attr + 10, "specular");
			shader_set_vec3(obj_shader, attr, point_lights[i].specular.raw);

			strcpy(attr + 10, "constant");
			shader_set_float(obj_shader, attr, point_lights[i].constant);

			strcpy(attr + 10, "linear");
			shader_set_float(obj_shader, attr, point_lights[i].linear);

			strcpy(attr + 10, "quadratic");
			shader_set_float(obj_shader, attr, point_lights[i].quadratic);
		}
		shader_set_int(obj_shader, "lights_num", LIGHTS_NUM);

		object_draw(&car, obj_shader);
		object_draw(&man, obj_shader);
		object_draw(&sorceress, obj_shader);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	object_free(&car);
	object_free(&man);
	for (int i = 0; i < 8; i++) {
		object_free(&point_light_objs[i]);
	}
	object_free(&sorceress);

	glDeleteProgram(obj_shader);
	glDeleteProgram(light_shader);

	glfwTerminate();

	return 0;
}
