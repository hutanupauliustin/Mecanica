#pragma once
#include "sistem.h"

enum {
    MOD_RULARE = 0,
    MOD_EDITARE,
    MOD_PLASARE_CORP,
    MOD_PLASARE_LEGATURA_1,
    MOD_PLASARE_LEGATURA_2
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

class editor{

    public:

    editor();

    int mod_curent;
    int cadru_activ;

    float mouse_x;
    float mouse_y;

    std::vector<int> corpuriSelectate;
    std::vector<fantomaUI> elementeUI;

    int corpApasat;

    int legatura_corpA;
    int stare_legatura;

    int gasesteCorpSubMouse(sistem &S);
    void mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY);
};
