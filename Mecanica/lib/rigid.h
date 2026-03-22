#pragma once
#include <cmath>

enum formaGeometrica{
    PUNCT,
    CERC,
    DREPTUNGHI,
    TRIUNGHI
};

struct material{
    float restituire = 0;
    float frecareStatica = 0;
    float frecareDinamica = 0 ;
    float frecareRostogolireStatica = 0;
    float frecareRostogolireDinamica = 0;
};

namespace materiale {
    //                              Restituire, Static, Dinamic, RostogStatic, RostogDinamic
    const material Cauciuc =          {0.8f,       0.9f,   0.7f,    0.05f,       0.02f};
    const material Gheata =           {0.1f,       0.05f,  0.01f,   0.01f,       0.007f};
    const material Lemn =             {0.4f,       0.5f,   0.4f,    0.02f,       0.05f};
    const material Piatra =           {0.2f,       0.8f,   0.6f,    0.04f,       0.01f};
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

class rigid
{
public:
    int index = 0; // Indexul corpului in vectorul sistemului
    bool activ = 1;
    
    float x, y, phi;
    float v_x, v_y, omega; // coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
    float f_x, f_y, moment;
    float M, J;
    
    geometrie collider;
    material material;
    
    rigid();
    rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie);
    
    void aflaForteProprii(float g);

    void coordPunctPeCorp(float &punct_x, float &punct_y, float d_x, float d_y);                //d_x si d_y sunt coordonatele punctului fata de sistemul de referinta mobil al corpului

    void vitezaPunctPeCorp(float &punct_v_x, float &punct_v_y, float d_x, float d_y);

    void seteazaBoundingBox();                  // "deseneaza" o cutie dreptunghiulara cu laturile paralele cu axele OX si OY ale sistemului, care sa cuprinda intreg rigidul
    
    // Creeaza o Bara (Dreptunghiulara)
    static rigid Bara( float x, float y, float Lungime, float Grosime, float Masa);

    static rigid Disc( float x, float y, float Raza, float Masa);

    // Creeaza un punct fix (Lumea)
    static rigid Fix( float x, float y);
};
