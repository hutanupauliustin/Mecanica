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
#include "input.h"
#include "editor.h"

GLFWwindow* openGLWindow(unsigned int &shaderProgram, unsigned int &gridProgram);

void initBuffers(unsigned int &VAO, unsigned int &VBO, unsigned int &gridVAO, unsigned int &gridVBO);

int updateVerticesData(sistem &S, editor &E, float* vertices);

void drawGrid(editor &E);

void drawSystem(sistem &S, editor &E, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer);