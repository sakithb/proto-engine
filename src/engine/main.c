#include <stdio.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "shader.h"
#include "camera.h"
#include "model.h"

void process_input(GLFWwindow *window);
void window_resize_cb(GLFWwindow *window, int width, int height);
void mouse_move_cb(GLFWwindow *window, double x, double y);

int scr_width = 1280;
int scr_height = 720;

float delta_time = 0.0f;
float last_frame = 0.0f;

struct camera cam;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "Proto Engine", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		fprintf(stderr, "Failed to create window\n");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
	glfwSetWindowSizeCallback(window, window_resize_cb);
	glfwSetCursorPosCallback(window, mouse_move_cb);

	if (!gladLoadGL(glfwGetProcAddress)) {
		glfwTerminate();
		fprintf(stderr, "Failed to initialize GLAD\n");
		return -1;
	}

	glViewport(0, 0, scr_width, scr_height);
	glEnable(GL_DEPTH_TEST);

	GLuint shader;
	shader_init(&shader, "assets/shaders/default.vert", "assets/shaders/default.frag");

	camera_init(&cam, (vec3s){0.0f, 3.0f, 10.0f});

	struct model obj;
	model_init(&obj, "assets/models/monk_character.model");

	while(!glfwWindowShouldClose(window)) {
		delta_time = glfwGetTime() - last_frame;
		last_frame = glfwGetTime();

		process_input(window);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader);

		mat4s model = GLMS_MAT4_IDENTITY;
		mat4s view = camera_lookat(&cam);
		mat4s projection = glms_perspective(45.0f, (float)scr_width / scr_height, 0.1f, 100.0f);

		shader_set_mat4(shader, "model", model.raw);
		shader_set_mat4(shader, "view", view.raw);
		shader_set_mat4(shader, "projection", projection.raw);

		model_draw(&obj, shader);

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	model_free(&obj);
	glDeleteProgram(shader);
	glfwTerminate();

	return 0;
}

void process_input(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera_move(&cam, CAM_FORWARD, delta_time);
	} else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera_move(&cam, CAM_BACKWARD, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera_move(&cam, CAM_LEFT, delta_time);
	} else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera_move(&cam, CAM_RIGHT, delta_time);
	}
}

double last_mouse_x = 0.0f;
double last_mouse_y = 0.0f;
int mouse_cb_first = 1;

void mouse_move_cb(GLFWwindow *window, double x, double y) {
	if (mouse_cb_first) {
		last_mouse_x = x;
		last_mouse_y = y;
		mouse_cb_first = 0;
	}

	camera_rotate(&cam, x - last_mouse_x, last_mouse_y - y);
	last_mouse_x = x;
	last_mouse_y = y;
};

void window_resize_cb(GLFWwindow *window, int width, int height) {
	scr_width = width;
	scr_height = height;
	glViewport(0, 0, scr_width, scr_height);
}
