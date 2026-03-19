#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "input.h"


void updateVerticesData(sistem &S, float* vertices);

GLFWwindow* openGLWindow(unsigned int &shaderProgram);

void initBuffers(unsigned int &VAO, unsigned int &VBO);

void drawSystem(sistem &S, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer);
