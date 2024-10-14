#include "camera.h"
#include "global.h"
#include "input.h"

void process_input(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera_move(&state.camera, CAM_FORWARD);
	} else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera_move(&state.camera, CAM_BACKWARD);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera_move(&state.camera, CAM_LEFT);
	} else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera_move(&state.camera, CAM_RIGHT);
	}
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
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

	camera_rotate(&state.camera, x - last_mouse_x, last_mouse_y - y);
	last_mouse_x = x;
	last_mouse_y = y;
};
