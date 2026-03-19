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

    // Variabila care tine minte cate arcuri am agatat de mouse in click-ul curent
    static int arcuriPuseDeMouse = 0; 

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        
        // 1. Tragem DOAR daca nu tragem deja de ceva (arcuriPuseDeMouse == 0)
        // si daca avem corpuri sub mouse.
        if (arcuriPuseDeMouse == 0 && S.corpuriSelectate.size() > 0) {
            
            for(int i = 0; i < S.corpuriSelectate.size(); i++) {
                int id_corp = S.corpuriSelectate[i];

                arc trage_spre_mouse = arc::Creaza(S.corpuri[S.id_corp_mouse], S.corpuri[id_corp], 
                                                   S.corpuri[S.id_corp_mouse].x, S.corpuri[S.id_corp_mouse].y,
                                                   S.corpuri[id_corp].x, S.corpuri[id_corp].y,
                                                   500.0f, 20.0f, 0.1f); // Am pus lungimea de repaus 0.1f ca sa se lipeasca de cursor, cu putina toleranta
                S.adaugaArcuri(trage_spre_mouse);
                
                arcuriPuseDeMouse++; // Numaram cate am adaugat
            }
        }
    } 
    else {
        // 2. Click-ul a fost eliberat. Avem arcuri de sters?
        if (arcuriPuseDeMouse > 0) {
            
            // Stergem fix atatea arcuri cate am pus (ele stau mereu la coada vectorului)
            for (int i = 0; i < arcuriPuseDeMouse; i++) {
                S.arcuri.pop_back();
            }
            
            // Resetam sistemul pentru urmatorul click
            arcuriPuseDeMouse = 0;
        }
    }


}