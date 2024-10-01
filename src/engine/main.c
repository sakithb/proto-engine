#include <stdio.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "cglm/struct.h"

int scr_width = 1280;
int scr_height = 720;

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

	if (!gladLoadGL(glfwGetProcAddress)) {
		glfwTerminate();
		fprintf(stderr, "Failed to initialize GLAD\n");
		return -1;
	}

	glViewport(0, 0, scr_width, scr_height);

	while(!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);
		glfwPollEvents();    
	}

	glfwTerminate();

	return 0;
}
