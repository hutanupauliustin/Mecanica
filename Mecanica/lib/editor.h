#pragma once
#include "sistem.h"
#include <vector>

enum {
    MOD_RULARE = 0,
    MOD_EDITARE,
    MOD_PLASARE_CORP,
    MOD_PLASARE_LEGATURA_1,
    MOD_PLASARE_LEGATURA_2
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
    bool arata_energie;
    bool arata_forte;
    bool distributie_viteze;
    bool distributie_acceleratie;
};

struct IstoricCorp{
    std::vector<float> axe[TOTAL_PARAMETRII];
    std::vector<float> timpAfisat;
    int offset = 0;
    int capacitate_maxima = 7200; // 60 FPS * 120 de secunde = 2 minute
};

class editor{

    public:

    editor();

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

    int legatura_corpA;
    int stare_legatura;

    double t;
    double dt;
    float scala_timp;

    editorFlags flag;

    std::vector<IstoricCorp> valoriSimulate; 

    int gasesteCorpSubMouse(sistem &S);
    void mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY);

    void sincronizeazaMemorie(sistem &S);
    void incarcaDatePentruGrafic(sistem &S);
};
