#include <stdio.h>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "shader.h"
#include "camera.h"
#include "model.h"

void process_input(GLFWwindow *window);
void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_resize_cb(GLFWwindow *window, int width, int height);
void mouse_move_cb(GLFWwindow *window, double x, double y);
void debug_msg_cb(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

int scr_width = 1280;
int scr_height = 720;

float delta_time = 0.0f;
float last_frame = 0.0f;

struct camera cam;
GLenum mode = GL_FILL;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  

	GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "Proto Engine", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		fprintf(stderr, "Failed to create window\n");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetWindowSizeCallback(window, window_resize_cb);
	glfwSetCursorPosCallback(window, mouse_move_cb);
	glfwSetKeyCallback(window, key_cb);

	if (!gladLoadGL(glfwGetProcAddress)) {
		glfwTerminate();
		fprintf(stderr, "Failed to initialize GLAD\n");
		return -1;
	}

	glViewport(0, 0, scr_width, scr_height);
	glEnable(GL_DEPTH_TEST);

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(debug_msg_cb, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	} 

	GLuint shader;
	shader_init(&shader, "assets/shaders/default.vert", "assets/shaders/default.frag");

	camera_init(&cam, (vec3s){0.0f, 3.0f, 10.0f});

	struct model obj;
	model_init(&obj, "assets/test_models/car.glb");

	while(!glfwWindowShouldClose(window)) {
		delta_time = glfwGetTime() - last_frame;
		last_frame = glfwGetTime();

		process_input(window);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, mode);

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

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		switch (mode) {
		case GL_FILL: mode = GL_LINE; break;
		case GL_LINE: mode = GL_POINT; break;
		case GL_POINT: mode = GL_FILL; break;
		}
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

void debug_msg_cb(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    printf("---------------\n");
    printf("Debug message (%s): \n", message);;

    switch (source) {
        case GL_DEBUG_SOURCE_API:             printf("Source: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("Source: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("Source: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     printf("Source: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER:           printf("Source: Other\n"); break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               printf("Type: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("Type: Undefined Behaviour\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         printf("Type: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         printf("Type: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              printf("Type: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          printf("Type: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           printf("Type: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               printf("Type: Other\n"); break;
    }
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         printf("Severity: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       printf("Severity: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          printf("Severity: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("Severity: notification\n"); break;
    }
}

