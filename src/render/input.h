#pragma once
#ifndef __EMSCRIPTEN__
  #include <glad/glad.h>
#endif

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
  #include <emscripten.h>
#endif  
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "editor.h"

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(sistem &S, editor &E);
