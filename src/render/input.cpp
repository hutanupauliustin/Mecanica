#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include "sistem.h"
#include "editor.h"
#include "input.h"
#include "imgui.h"
#include "instrument.h"

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    editor* E = static_cast<editor*>(glfwGetWindowUserPointer(window));
    if (!E) return;
    
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;
    
    (void)xoffset;
    E->camera.zoom -= (float)yoffset * 0.5f;
    if (E->camera.zoom < 1.0f)   E->camera.zoom = 1.0f;
    if (E->camera.zoom > 100.0f) E->camera.zoom = 100.0f;
}

void actualizeazaSubMouseVizual(sistem& S, editor& E) {

    for(size_t i = 0; i < E.corpuriSubMouse.size(); i ++)
        S.corpuri[E.corpuriSubMouse[i]].collider.subMouse = 0;

    int id = E.gasesteCorpSubMouse(S);
    if (id != -1) {
        S.corpuri[id].collider.subMouse = 1;
    }
    }

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    editor* E = static_cast<editor*>(glfwGetWindowUserPointer(window));
    
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    
    if (E) E->camera.aspect_ratio = (float)width / (float)height;
}

void processInput(sistem &S, editor &E) {
    
    E.updateMousePosition();    
    actualizeazaSubMouseVizual(S,E);

    if (E.instrumentCurent) {
        
        static bool stangaApasat = false;
        if (glfwGetMouseButton(E.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!stangaApasat) {
                E.instrumentCurent->clickStanga(S, E, E.mouse_x, E.mouse_y);
                stangaApasat = true;
            } else {
                E.instrumentCurent->miscareMouse(S, E, E.mouse_x, E.mouse_y);
            }
        } else {
            if (stangaApasat) {
                E.instrumentCurent->eliberareClickStanga(S, E);
                stangaApasat = false;
            }
        }

        static bool dreaptaApasat = false;
        if (glfwGetMouseButton(E.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (!dreaptaApasat) {
                E.instrumentCurent->clickDreapta(S, E);
                dreaptaApasat = true;
            }
        } else {
            dreaptaApasat = false;
        }

        E.instrumentCurent->pregatesteFantome(E.elementeUI, E.mouse_x, E.mouse_y, S);
    }
}


