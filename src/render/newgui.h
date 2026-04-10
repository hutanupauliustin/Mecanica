#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include "grafica.h"
#include "font.h"
#include <cstring>
#include "portable-file-dialogs.h"
#include "scena.h"

void setupFont(ImGuiIO& io);
void setupGUI(GLFWwindow* window);
void cleanupGUI();

void startFrameGUI();
void endFrameGUI();

void renderMeniu(sistem &S, editor &E);

void renderPanouInstrumente(sistem &S, editor &E);

void renderInspector(sistem &S, editor &E);

void renderOverlayStatus(sistem &S, editor &E);

void renderToatePanourile(sistem &S, editor &E);