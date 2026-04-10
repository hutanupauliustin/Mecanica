#pragma once
#include "sistem.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>

enum ModEditor {
    MOD_RULARE = 0,
    MOD_EDITARE = 1,
    MOD_ADAUGARE_CORP = 2,
    MOD_ADAUGARE_ARC_PAS_1 = 3,
    MOD_ADAUGARE_ARC_PAS_2 = 4,
    MOD_SETARE_ARC = 5,
    MOD_ADAUGARE_LEGATURA_PAS_1 = 6,
    MOD_ADAUGARE_LEGATURA_PAS_2 = 7
};

enum{
    POZITIE_X = 0,
    POZITIE_Y,
    POZITIE_PHI,
    VITEZA_X,
    VITEZA_Y,
    VITEZA_OMEGA,
    ACCELERATIE_X,
    ACCELERATIE_Y,
    ACCELERATIE_EPSILON,
    TOTAL_PARAMETRII
};

struct fantomaUI {
    bool activa = false;
    float x = 0.0f;
    float y = 0.0f;
    float phi = 0.0f;
    int tip; // 0 = Punct, 1 = Cerc, 2 = Dreptunghi 
    float dim1;
    float dim2;
    culoare col;
};

struct editorFlags {
    bool arata_energie = false;
    bool arata_forte = false;
    bool distributie_viteze = false;
    bool distributie_acceleratie = false;
};

struct IstoricCorp{
    std::vector<float> axe[TOTAL_PARAMETRII];
    std::vector<float> timpAfisat;
    int offset = 0;
    int capacitate_maxima = 7200; // 60 FPS * 120 de secunde = 2 minute

    IstoricCorp() {
        timpAfisat.reserve(capacitate_maxima);
        for(size_t i = 0; i < TOTAL_PARAMETRII; i++) {
            axe[i].reserve(capacitate_maxima);
        }
    }

};

class editor{

    public:

    editor();
    ~editor();

    int cadru_activ;
    int mod_vizualizare; //0 -- normal 1--distributia vitezelor 2-- distributia acceleratiilor

    float mouse_x;
    float mouse_y;

    struct camera {
        float x = 0.0f;
        float y = 0.0f;
        float zoom = 10.0f;
        float aspect_ratio = 1.33f;

        vec2 screenToWorld(float mouseX, float mouseY, int screenWidth, int screenHeight) {
            float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
            float ndcY = 1.0f - (2.0f * mouseY) / screenHeight; 
            return vec2(ndcX * zoom * aspect_ratio + x, ndcY * zoom + y);
        }
    };

    std::vector<int> corpuriSubMouse;
    std::vector<int> corpuriSelectate;
    std::vector<int> legaturiSelectate;
    std::vector<fantomaUI> elementeUI;

    unsigned int VAO, VBO;
    std::vector<float> vertexBuffer;
    size_t total_elemente;

    unsigned int shaderProgram;
    unsigned int vertexStride;
    unsigned int frameCount;

    editorFlags flag;

    std::vector<IstoricCorp> valoriSimulate; 

    std::string nume_fisier_export = ".temp_istoric_simulare.csv";
    std::ofstream fisier_export;

    int gasesteCorpSubMouse(sistem &S);
    void mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY);

    void sincronizeazaMemorie(sistem &S);
    void incarcaDatePentruGrafic(sistem &S);

    void updateMousePosition(GLFWwindow *window);
    
};
