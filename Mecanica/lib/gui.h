#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "sistem.h"
#include "editor.h"
#include "implot.h"

void setupGUI(GLFWwindow* window);
void startFrameGUI();
void renderPanouDeControl(sistem &S, editor &E);
void renderInspector(sistem &S, editor &E);
void endFrameGUI();
void cleanupGUI();
GLFWwindow* initializareGrafica(sistem &S, editor &E);
void randareGrafica(sistem &S, editor &E, GLFWwindow* &window);