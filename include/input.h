#ifndef INPUT_H
#define INPUT_H

#include "GLFW/glfw3.h"

void process_input(GLFWwindow *window);

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_cb(GLFWwindow *window, double x, double y);

#endif
