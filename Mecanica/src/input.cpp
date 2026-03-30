#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "editor.h"
#include "input.h"

float zoomScale = 10.0f;
float cameraX = 0.0f;
float cameraY = 0.0f;
float aspect_ratio = 800.0f / 600.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
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

void processInput(GLFWwindow *window, float &dt, sistem &S, editor &E) {
    //inchide programul
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //creste viteza 
    static bool plusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        if(!plusApasat){
            dt += 0.001f; 
            if(dt > 0.01f) dt = 0.01f;
            plusApasat = true;
        }
    } else {
        plusApasat = false;
    }

    // scade viteza
    static bool minusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        if(!minusApasat){
            dt -= 0.001f; 
            if(dt < 0.0001f) dt = 0.0001f;
            minusApasat = true;
        }
    } else {
        minusApasat = false;
    }

    // Pauza
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

    // Trimitem mouse-ul real catre Panoul de Bord (editor)
    E.mouse_x = mouseX;
    E.mouse_y = mouseY;

    for (int i = 0; i < S.corpuri.size(); i++) { S.corpuri[i].collider.selectat = 0; }
    E.corpuriSelectate.clear();

    // Editorul cauta corpul acum, nu fisierul de input
    int id_sub_mouse = E.gasesteCorpSubMouse(S);
    if (id_sub_mouse != -1) {
        S.corpuri[id_sub_mouse].collider.selectat = 1;
        E.corpuriSelectate.push_back(id_sub_mouse);
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        switch(E.mod_curent){
            case MOD_RULARE:
                if(id_sub_mouse != -1)
                    E.corpApasat = id_sub_mouse;
                    
                break;
            case MOD_EDITARE:               
                if(id_sub_mouse != -1) {
                    float offsetx = S.corpuri[id_sub_mouse].pozitie.x - E.mouse_x;
                    float offsety = S.corpuri[id_sub_mouse].pozitie.y - E.mouse_y;
                    E.mutaCorp(S, id_sub_mouse, offsetx, offsety);
                }
                break;

            case MOD_PLASARE_CORP:          
            case MOD_PLASARE_LEGATURA_1:    
            case MOD_PLASARE_LEGATURA_2:    
                break;
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
        switch(E.mod_curent){
            case MOD_RULARE:
                E.corpApasat = -1;
                    
                break;
            case MOD_EDITARE:               
            case MOD_PLASARE_CORP:          
            case MOD_PLASARE_LEGATURA_1:    
            case MOD_PLASARE_LEGATURA_2:    
                break;
        }
    }
}