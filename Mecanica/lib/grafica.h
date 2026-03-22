#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "input.h"

enum{
    INTERACTIONARE = 0,
    EDITARE
};

void updateVerticesData(sistem &S, float* vertices);

GLFWwindow* openGLWindow(unsigned int &shaderProgram);

void initBuffers(unsigned int &VAO, unsigned int &VBO);

void drawSystem(sistem &S, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer);

// Functii GUI
void setupGUI(GLFWwindow* window);
void startFrameGUI();
void renderPanouDeControl(float &dt, bool &running_flag, bool &arata_energie_flag, float t, float energie);
void renderPanouDeAdaugatCorpuri(sistem &S);
void endFrameGUI();
void cleanupGUI();
