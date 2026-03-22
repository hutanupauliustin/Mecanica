#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "input.h"

float zoomScale = 10.0f;
float cameraX = 0.0f;
float cameraY = 0.0f;
float aspect_ratio = 800.0f / 600.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset este +1 când dai scroll în sus (zoom in) și -1 când dai scroll în jos (zoom out)
    zoomScale -= (float)yoffset * 0.5f; // 0.5f este sensibilitatea (cât de rapid faci zoom)
    
    // Punem limite pentru a nu da zoom prea aproape și pentru a evita împărțirea la 0 (sau numere negative care întorc ecranul cu capul în jos)
    if (zoomScale < 1.0f) 
        zoomScale = 1.0f; 
    if (zoomScale > 100.0f) 
        zoomScale = 100.0f; 
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)   //modifica marimea ecranului cand se modifica marinea ferestrei
{
        if (height == 0) height = 1; // Evitam impartirea la 0
        glViewport(0, 0, width, height);
        aspect_ratio = (float)width / (float)height; 
        
}

void processInput(GLFWwindow *window, float &dt, bool &running_flag, sistem &S) {
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
            running_flag = !running_flag; 
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

    static double mx_anterior = mx;
    static double my_anterior = my;

    float ndcX = (2.0f * (float)mx) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)my) / height; // Inversam axa Y a ecranului
    
    float mouseX = ndcX * zoomScale*aspect_ratio + cameraX;
    float mouseY = ndcY * zoomScale + cameraY;      //transforma coordonatele mouselui din pixeli in "metri"

    S.corpuri[S.id_corp_mouse].x = mouseX;
    S.corpuri[S.id_corp_mouse].y = mouseY;

    S.stare(S.id_corp_mouse*3,0) = mouseX;
    S.stare(S.id_corp_mouse*3 + 1,0) = mouseY;


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {            // pentru a ne muta prin cadru
        double deltaX = mx - mx_anterior;
        double deltaY = my - my_anterior;
        
        // Transformam pixelii parcursi de mouse in unitati fizice
        cameraX -= (float)(deltaX / width) * 2.0f * zoomScale;
        cameraY += (float)(deltaY / height) * 2.0f * zoomScale; // += pentru ca Y-ul de la mouse e invers
    }

    mx_anterior = mx;
    my_anterior = my;

    // Variabile statice pentru dragging si arcuri
    static int arcuriPuseDeMouse = 0; 
    static int corp_dragged_id = -1;
    static float offset_x = 0.0f;
    static float offset_y = 0.0f;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        
        if (S.mod_curent == 1) {
            // --- MOD EDITARE: Mutare directa (fara arcuri) ---
            if (corp_dragged_id == -1 && S.corpuriSelectate.size() > 0) {
                int id = S.corpuriSelectate[0]; // Luam corpul de sub mouse
                if (id > 1) { // Excludem Lumea (0) si Mouse-ul virtual (1)
                    corp_dragged_id = id;
                    offset_x = S.corpuri[id].x - mouseX;
                    offset_y = S.corpuri[id].y - mouseY;
                }
            }
            
            // Daca avem un corp prins, ii suprascriem pozitia si oprim inertiile
            if (corp_dragged_id != -1) {
                S.corpuri[corp_dragged_id].x = mouseX + offset_x;
                S.corpuri[corp_dragged_id].y = mouseY + offset_y;
                S.corpuri[corp_dragged_id].v_x = 0.0f;
                S.corpuri[corp_dragged_id].v_y = 0.0f;
                S.corpuri[corp_dragged_id].omega = 0.0f;
                
                // Actualizam matricea direct ca fizica sa nu-l arunce inapoi
                S.stare(corp_dragged_id * 3, 0) = S.corpuri[corp_dragged_id].x;
                S.stare(corp_dragged_id * 3 + 1, 0) = S.corpuri[corp_dragged_id].y;
            }
        } 
        else {
            // --- MOD INTERACTIUNE: Tragere cu arcuri ---
            if (arcuriPuseDeMouse == 0 && S.corpuriSelectate.size() > 0) {
                for(int i = 0; i < S.corpuriSelectate.size(); i++) {
                    int id_corp = S.corpuriSelectate[i];
                    if (id_corp > 1) {
                        arc trage_spre_mouse = arc::Creaza(S.corpuri[S.id_corp_mouse], S.corpuri[id_corp], 
                                                           mouseX, mouseY, 
                                                           mouseX, mouseY, 
                                                           800.0f, 60.0f, 0.0f);
                        S.adaugaArcuri(trage_spre_mouse);
                        arcuriPuseDeMouse++; 
                    }
                }
            }
        }
    } 
    else {
        // --- CLICK ELIBERAT ---
        corp_dragged_id = -1; // Resetam mutarea directa
        
        if (arcuriPuseDeMouse > 0) {
            int stersi = 0;
            for (int i = S.arcuri.size() - 1; i >= 0 && stersi < arcuriPuseDeMouse; i--) {
                if (S.arcuri[i].activ && S.arcuri[i].contorCorpA == S.id_corp_mouse) {
                    S.eliminaArc(i);
                    stersi++;
                }
            }
            arcuriPuseDeMouse = 0;
        }
    }


}