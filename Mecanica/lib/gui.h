#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "sistem.h"
#include "editor.h"

void setupGUI(GLFWwindow* window);
void startFrameGUI();
void renderPanouDeControl(float &dt, bool &running_flag, bool &arata_energie_flag, float t, float energie);
void renderPanouDeAdaugatCorpuri(sistem &S, editor &E);
void endFrameGUI();
void cleanupGUI();