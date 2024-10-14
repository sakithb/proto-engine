#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "global.h"
#include "input.h"
#include "setup.h"

void window_resize_cb(GLFWwindow *window, int width, int height);
void debug_msg_cb(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *param);

GLFWwindow *setup() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  

	GLFWwindow *window = glfwCreateWindow(state.scr_width, state.scr_height, "Proto Engine", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		fprintf(stderr, "Failed to create window\n");
		abort();
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
		abort();
	}

	glViewport(0, 0, state.scr_width, state.scr_height);
	glEnable(GL_DEPTH_TEST);

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(debug_msg_cb, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	} 

	return window;
}

void window_resize_cb(GLFWwindow *window, int width, int height) {
	state.scr_width = width;
	state.scr_height = height;
	glViewport(0, 0, state.scr_width, state.scr_height);
}

void debug_msg_cb(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *param) {
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

