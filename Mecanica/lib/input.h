#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "editor.h"

extern float zoomScale;
extern float cameraX;
extern float cameraY;
extern float aspect_ratio;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow *window, float &dt, sistem &S, editor &E);
