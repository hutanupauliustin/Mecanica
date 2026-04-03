#pragma once
#include "sistem.h"
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

    int mod_curent;
    int cadru_activ;

    int mod_vizualizare; //0 -- normal 1--distributia vitezelor 2-- distributia acceleratiilor

    float mouse_x;
    float mouse_y;

    std::vector<int> corpuriSubMouse;
    std::vector<int> corpuriSelectate;
    std::vector<fantomaUI> elementeUI;

    unsigned int VAO, VBO;
    std::vector<float> vertexBuffer;
    size_t total_elemente;

    unsigned int shaderProgram;
    unsigned int vertexStride;
    unsigned int frameCount;

    int adaugare_corp_A = 0;
    int adaugare_corp_B = 0;
    vec2 adaugare_punct_A_local;
    vec2 adaugare_punct_B_local;
    int tip_legatura_de_adaugat = 0; // 0 = Articulatie, 1 = Incastrare

    float arc_l0_procent = 100.0f;
    float arc_k = 100.0f;
    float arc_d = 5.0f;

    double t;
    double dt;
    float scala_timp;

    editorFlags flag;

    std::vector<IstoricCorp> valoriSimulate; 

    int gasesteCorpSubMouse(sistem &S);
    void mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY);

    void sincronizeazaMemorie(sistem &S);
    void incarcaDatePentruGrafic(sistem &S);
    
    std::string nume_fisier_export = ".temp_istoric_simulare.csv";
    std::ofstream fisier_export;
};
