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
    
    for (const auto& el : E.elementeSubMouse) {
        if (el.tip == TIP_CORP && el.id >= 0) S.corpuri[el.id].collider.subMouse = 0;
        else if (el.tip == TIP_LEGATURA && el.id >= 0) S.legaturi[el.id]->subMouse = 0;
    }
    E.elementeSubMouse.clear();

    ObiectSelectat sub_cursor = E.gasesteObiectSubMouse(S);

    if (sub_cursor.id != -1) {
        if (sub_cursor.tip == TIP_CORP) {
            S.corpuri[sub_cursor.id].collider.subMouse = 1;
        } else if (sub_cursor.tip == TIP_LEGATURA) {
            S.legaturi[sub_cursor.id]->subMouse = 1;
        }
        
        E.elementeSubMouse.push_back(sub_cursor);
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
                if(!ImGui::GetIO().WantCaptureMouse){
                    E.instrumentCurent->clickStanga(S, E, E.mouse_x, E.mouse_y);
                    stangaApasat = true;
                }
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
                if(!ImGui::GetIO().WantCaptureMouse){
                    E.instrumentCurent->clickDreapta(S, E);
                    dreaptaApasat = true;
                }
            }
        } else {
            dreaptaApasat = false;
        }

        E.instrumentCurent->pregatesteFantome(E.elementeUI, E.mouse_x, E.mouse_y, S);
    }
}


