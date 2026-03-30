#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "sistem.h"
#include "editor.h"

void setupGUI(GLFWwindow* window);
void startFrameGUI();
void renderPanouDeControl(sistem &S,editor &E, float &dt, float t, float energie);
void renderInspector(sistem &S, editor &E);
void endFrameGUI();
void cleanupGUI();