#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "input.h"
#include "editor.h"

GLFWwindow* openGLWindow(unsigned int &shaderProgram);

void initBuffers(unsigned int &VAO, unsigned int &VBO);

int updateVerticesData(sistem &S, editor &E, float* vertices, bool arata_forte);

void drawSystem(sistem &S, editor &E, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer);