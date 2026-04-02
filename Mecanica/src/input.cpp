#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include "sistem.h"
#include "editor.h"
#include "input.h"
#include "imgui.h"

float zoomScale = 10.0f;
float cameraX = 0.0f;
float cameraY = 0.0f;
float aspect_ratio = 800.0f / 600.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse)
        return;
    zoomScale -= (float)yoffset * 0.5f; 
    
    if (zoomScale < 1.0f) zoomScale = 1.0f; 
    if (zoomScale > 100.0f) zoomScale = 100.0f; 
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (height == 0) height = 1; 
    glViewport(0, 0, width, height);
    aspect_ratio = (float)width / (float)height; 
}

void processInput(GLFWwindow *window, sistem &S, editor &E) {

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool plusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        if(!plusApasat){
            E.dt += 0.001f; 
            if(E.dt > 0.01f) E.dt = 0.01f;
            plusApasat = true;
        }
    } else {
        plusApasat = false;
    }

    static bool minusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        if(!minusApasat){
            E.dt -= 0.001f; 
            if(E.dt < 0.0001f) E.dt = 0.0001f;
            minusApasat = true;
        }
    } else {
        minusApasat = false;
    }

    static bool spaceApasat = false;

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        if(!spaceApasat) {
           E.mod_curent = E.mod_curent != MOD_RULARE ?  MOD_RULARE : MOD_EDITARE;
            spaceApasat = true;
            std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
        }
    } else {
        spaceApasat = false; 
    }

    //-------Mouse Input------//
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float ndcX = (2.0f * (float)mx) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)my) / height; 
    
    float mouseX = ndcX * zoomScale * aspect_ratio + cameraX;
    float mouseY = ndcY * zoomScale + cameraY;      

    ImGuiIO& io = ImGui::GetIO();

    static float ndcX_anterior = ndcX;
    static float ndcY_anterior = ndcY;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS && !io.WantCaptureMouse) {
        float delta_camX = (ndcX - ndcX_anterior) * zoomScale * aspect_ratio;
        float delta_camY = (ndcY - ndcY_anterior) * zoomScale;
        
        cameraX -= delta_camX;
        cameraY -= delta_camY;
    }

    ndcX_anterior = ndcX;
    ndcY_anterior = ndcY;

    mouseX = ndcX * zoomScale * aspect_ratio + cameraX;
    mouseY = ndcY * zoomScale + cameraY;

    E.mouse_x = mouseX;
    E.mouse_y = mouseY;

    for(int i = 0; i <  E.corpuriSubMouse.size(); i++)
        S.corpuri[E.corpuriSubMouse[i]].collider.subMouse = 0;
    E.corpuriSubMouse.clear();
    

    int id_sub_mouse = E.gasesteCorpSubMouse(S);
    if (id_sub_mouse != -1) {
        S.corpuri[id_sub_mouse].collider.subMouse = 1;
        E.corpuriSubMouse.push_back(id_sub_mouse);
    }

    static bool stangaApasat = false;
    bool stangaClick = false;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!stangaApasat) {
            stangaClick = true; 
            stangaApasat = true;
        }
    } else {
        stangaApasat = false;
    }

    bool hasShift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if(stangaClick && !io.WantCaptureMouse){
        if(id_sub_mouse != -1){
            
            bool dejaSelectat = ( std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), id_sub_mouse) != E.corpuriSelectate.end());
            if (hasShift) {
                if (dejaSelectat) {
                    S.corpuri[id_sub_mouse].collider.selectat = 0;
                    E.corpuriSelectate.erase(std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), id_sub_mouse));
                } else {
                    S.corpuri[id_sub_mouse].collider.selectat = 1;
                    E.corpuriSelectate.push_back(id_sub_mouse);
                }
            } else {
                if (!dejaSelectat) { 
                    for (int i = 0 ; i < E.corpuriSelectate.size(); i++) {
                        S.corpuri[E.corpuriSelectate[i]].collider.selectat = 0;
                    }
                    E.corpuriSelectate.clear();
                    
                    S.corpuri[id_sub_mouse].collider.selectat = 1;
                    E.corpuriSelectate.push_back(id_sub_mouse);
                }
            }
        
        } else {
            if(!hasShift) {
                for (int i = 0 ; i < E.corpuriSelectate.size(); i++) {
                    S.corpuri[E.corpuriSelectate[i]].collider.selectat = 0;
                }
                E.corpuriSelectate.clear();
            }
        }
    }
    static bool seMutaCorpuri = false;
    static vec2 pozitieMouseTrecut(mouseX, mouseY);
    vec2 mouseCurent(mouseX, mouseY);

    if(stangaClick && id_sub_mouse != -1) {
        if(std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), id_sub_mouse) != E.corpuriSelectate.end()) {
            seMutaCorpuri = true;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (E.mod_curent == MOD_EDITARE && seMutaCorpuri) {
            
            float deltaX = mouseCurent.x - pozitieMouseTrecut.x;
            float deltaY = mouseCurent.y - pozitieMouseTrecut.y;

            for (int id : E.corpuriSelectate) {
                S.corpuri[id].pozitie.x += deltaX;
                S.corpuri[id].pozitie.y += deltaY;

                S.corpuri[id].viteza.x = 0.0f;
                S.corpuri[id].viteza.y = 0.0f;
                S.corpuri[id].omega = 0.0f;
            }

            if (E.corpuriSelectate.size() > 0 && (deltaX != 0 || deltaY != 0)) {
                
                for(int i = 0; i < S.corpuri.size(); i++){
                    S.corpuri[i].forte_desen.reseteaza();
                }
                S.seteazaForteExterne();
                S.incarcaStare();
            }
        }
    } else {
        seMutaCorpuri = false; 
    }
    pozitieMouseTrecut = mouseCurent;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        for (int i = 0 ; i < E.corpuriSelectate.size(); i++) {
            S.corpuri[E.corpuriSelectate[i]].collider.selectat = 0;
        }
        E.corpuriSelectate.clear();
    }
}
