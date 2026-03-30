#pragma once
#include <cmath>
#include <vector>
#include "matrice.h"

enum formaGeometrica{
    PUNCT,
    CERC,
    DREPTUNGHI,
    TRIUNGHI
};

struct tipMaterial{
    float restituire = 0;
    float frecareStatica = 0;
    float frecareDinamica = 0 ;
    float frecareRostogolireStatica = 0;
    float frecareRostogolireDinamica = 0;
};

namespace materiale {
    //                              Restituire, Static, Dinamic, RostogStatic, RostogDinamic
    const tipMaterial Cauciuc =          {0.8f,       0.9f,   0.7f,    0.05f,       0.02f};
    const tipMaterial Gheata =           {0.1f,       0.05f,  0.01f,   0.01f,       0.007f};
    const tipMaterial Lemn =             {0.4f,       0.5f,   0.4f,    0.02f,       0.05f};
    const tipMaterial Piatra =           {0.2f,       0.8f,   0.6f,    0.04f,       0.01f};
}

struct boundingBox{
    float razaInaltime;
    float razaLatime;
};

struct culoare{
    float r, g, b, a;
};

struct geometrie{
    formaGeometrica tip;
    culoare culoare = {1.0f, 1.0f, 1.0f, 1.0f};
    float dimensiune1; // Raza pentru CERC, Latime pentru DREPTUNGHI
    float dimensiune2; // Lungime pentru DREPTUNGHI, este egala cu dimensiun1 in cazul cercului
    boundingBox bb;
    bool obiectStatic = 0;
    bool obiectVirtual = 0;
    bool selectat = 0;
    int cadru = 0;

    float coeficientAerodinamic = 0;
};

struct FantomaUI {
    bool activ = false;
    float x = 0.0f;
    float y = 0.0f;
    float phi = 0.0f;
    int tip_forma; // 0 = Punct, 1 = Cerc, 2 = Dreptunghi, 3 = Triunghi, 4 = Arc, 5 = Sageata
    float dimensiune1;
    float dimensiune2;
    culoare col;
};

struct torsor{
        vec2 forta;
        float moment = 0;
};

struct fortaExterna{
    vec2 u;                         //coordonatele versorului suport
    vec2 punct_aplicatie;           //coordonatele punctului de aplicare in coordonate globale
    float modul;
};

enum TipFortaVizuala {
    FORTA_GREUTATE,
    FORTA_ELASTICA,
    FORTA_REACTIUNE,
    FORTA_IMPACT_NORMAL,
    FORTA_IMPACT_FRECARE
};

struct fortaVizuala {
    TipFortaVizuala tip;
    vec2 valoare;
    vec2 punct_aplicare; 
};

struct debugForte {
    std::vector<fortaVizuala> forte;

    void reseteaza() {
        forte.clear(); 
    }
};

class rigid
{
public:
    int index = 0; // Indexul corpului in vectorul sistemului
    bool activ = 1;
    
    vec2 pozitie;
    float  phi;
    vec2 viteza;
    float omega; // coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
    torsor tau;
    float M, J;
    
    geometrie collider;
    tipMaterial material;

    std::vector<fortaExterna> forte;
    debugForte forte_desen;

    
    rigid();
    rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie);
    
    void adauagaForte(float modul_forta, float x_aplicare, float y_aplicare, float u_x, float u_y);

    void aflaForteProprii(float g);

    void reducereTorsor();

    vec2 localToGlobal(vec2 punctLocal);
    
    vec2 globalToLocal(vec2 punctGlobal);

    vec2 vitezaAbsolutaPunct(vec2 punctLocal);

    void seteazaBoundingBox();                  // "deseneaza" o cutie dreptunghiulara cu laturile paralele cu axele OX si OY ale sistemului, care sa cuprinda intreg rigidul
    
    static rigid Bara( float x, float y, float Lungime, float Grosime, float Masa, const char* numeMaterial = "Lemn");

    static rigid Disc( float x, float y, float Raza, float Masa, const char* numeMaterial = "Lemn");

    static rigid Fix( float x, float y);
};
