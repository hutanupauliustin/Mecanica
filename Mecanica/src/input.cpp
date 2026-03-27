#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"
#include "input.h"

float zoomScale = 10.0f;
float cameraX = 0.0f;
float cameraY = 0.0f;
float aspect_ratio = 800.0f / 600.0f;

void mutaCorp(sistem &S, int idCorp, float mx, float my, int offsetX, int offsetY){   

    S.corpuri[idCorp].x = mx + offsetX;
    S.corpuri[idCorp].y = my + offsetY;

}


int gasesteCorpSubMouse(sistem &S, float mouseX, float mouseY){
    int celMaiAproape_id = -1;
    float min_distanta = 999999.9f;

    for (int k = S.corpuri.size() - 1; k >= 1; k--) { 
        if (!S.corpuri[k].activ) continue;
        if (S.corpuri[k].collider.obiectVirtual) continue; // Nu vrem să selectăm fantomele
        
        // Ignorăm corpul A deja selectat, pentru a putea găsi corpul B aflat sub el
        if (S.mod_curent == MOD_PLASARE_LEGATURA_1 && S.legatura_corp_A == k) continue;

        // În modul Editare, putem impune să selectăm doar corpurile din layerul activ
        if (S.mod_curent == MOD_EDITARE && S.corpuri[k].collider.cadru != S.cadru_activ) continue;

        rigid &target = S.corpuri[k];
        float dx = mouseX - target.x;
        float dy = mouseY - target.y;
        
        float localX = dx * std::cos(target.phi) + dy * std::sin(target.phi);
        float localY = -dx * std::sin(target.phi) + dy * std::cos(target.phi);

        bool lovit = false;
        float cur_dist = 999999.0f;

        if (target.collider.tip == DREPTUNGHI) {
            float hw = target.collider.dimensiune1 / 2.0f;
            float hh = target.collider.dimensiune2 / 2.0f;
            // Adăugăm un halo subtil doar în modul editare
            float halo = (S.mod_curent == 1) ? std::max(0.15f, std::min(hw, hh) * 0.5f) : 0.0f;

            if (std::abs(localX) <= hw + halo && std::abs(localY) <= hh + halo) {
                lovit = true;
                cur_dist = std::sqrt(dx*dx + dy*dy);
            }
        } else if (target.collider.tip == CERC) {
            float R = target.collider.dimensiune1;
            float halo = (S.mod_curent == 1) ? std::max(0.15f, R * 0.3f) : 0.0f;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist <= R + halo) {
                lovit = true;
                cur_dist = dist;
            }
        }

        if (lovit) {
            if (celMaiAproape_id == -1 || cur_dist < min_distanta - 0.1f) {
                min_distanta = cur_dist;
                celMaiAproape_id = k;
            }
        }
    }

    return celMaiAproape_id;
}

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

    // Trimitem mouse-ul real catre GUI
    S.mouse_x = mouseX;
    S.mouse_y = mouseY;

    for (int i = 0; i < S.corpuri.size(); i++) { S.corpuri[i].collider.selectat = 0; }
    S.corpuriSelectate.clear();

    int id_sub_mouse = gasesteCorpSubMouse(S, mouseX, mouseY);
    if (id_sub_mouse != -1) {
        S.corpuri[id_sub_mouse].collider.selectat = 1;
        S.corpuriSelectate.push_back(id_sub_mouse);
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        switch(S.mod_curent){
            case MOD_RULARE :               //interactiune cu corpuri prin spring-uri cu un deadzone pentru a evita miscare haotica
                                arc a;
                                S.adaugaArcuri(a);
            case MOD_EDITARE:               //mutarea corpurilor direct
                                float offsetx =  S.corpuri[id_sub_mouse].x - mx;
                                float offsety =  S.corpuri[id_sub_mouse].y - my;
                                mutaCorp(S,id_sub_mouse,mouseX,mouseY,offsetx, offsety);
                                break;

            case MOD_PLASARE_CORP:          //creaza o fantoma peste mouse care arata ca corpul pe care il pune
            case MOD_PLASARE_LEGATURA_1:    //creaza o fantoma peste mouse cu legatura pe care vrem sa o punem, si asteapta sa apasam pe un corp care va fii primul corp din legatura
            case MOD_PLASARE_LEGATURA_2:    //tine lipit de mouse fantoma legaturii si fantoma corpului 1, si asteapta sa selectam un al doilea corp ca sa completam legatura 
            break;
        
        
        }
    }
}
