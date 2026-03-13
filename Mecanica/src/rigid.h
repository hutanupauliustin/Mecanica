#pragma once
#include <cmath>

enum formaGeometrica{
    PUNCT,
    CERC,
    DREPTUNGHI,
    TRIUNGHI
};

struct boundingBox{
    float razaInaltime;
    float razaLatime;
};

struct geometrie{
    formaGeometrica tip;
    float dimensiune1; // Raza pentru CERC, Latime pentru DREPTUNGHI
    float dimensiune2; // Lungime pentru DREPTUNGHI, este egala cu dimensiun1 in cazul cercului
    boundingBox bb;
    bool obiectStatic = 0;
    int layer = 0;


};



class rigid
{
public:
    int index = 0; // Indexul corpului in vectorul sistemului
    
    float x, y, phi;
    float v_x, v_y, omega; // coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
    float f_x, f_y, moment;
    float M, J;
    
    geometrie collider;
    
    rigid() :  x(0), y(0), phi(0), v_x(0), v_y(0), omega(0), M(1), J(1), f_x(0), f_y(0){
        collider.tip = PUNCT;
        collider.dimensiune1 = 1.0f;
        collider.dimensiune2 = 1.0f;
        collider.bb.razaInaltime = 1.0f;
        collider.bb.razaLatime = 1.0f;
    }

    rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie)
        :  x(x_initial), y(y_initial), phi(phi_initial), M(masa), J(momentInertie), v_x(0), v_y(0), omega(0), f_x(0), f_y(0), moment(0){}

    void aflaForteProprii(float g, float k_a)
    {
        // Daca masa este foarte mare (infinita), consideram corpul fix si nu ii aplicam greutate
        // pentru a evita erori matematice (Infinity * ceva = NaN)
        if(M > 1e10f) {
            f_x = 0;
            f_y = 0;
            moment =0;
        } else {
            f_x = 0;
            f_y = -g * M; // Forta gravitationala

            f_x += k_a * v_x;
            f_y += k_a * v_y;
            moment = 0;
        }
    }

    void coordPunctPeCorp(int &punct_x, int &punct_y, int d_x, int d_y){                //d_x si d_y sunt coordonatele punctului fata de sistemul de referinta mobil al corpului
        punct_x = x + d_x* cos(phi) - d_y * sin(phi);
        punct_y = y + d_x* sin(phi) + d_y * cos(phi);
    }
    
    void seteazaBoundingBox(){                  // "deseneaza" o cutie dreptunghiulara cu laturile paralele cu axele OX si OY ale sistemului, care sa cuprinda intreg rigidul
        
        if(collider.tip == PUNCT && collider.obiectStatic == 0){
            return;
        }

        if(collider.tip == CERC){
            collider.bb.razaLatime = collider.dimensiune1;
            collider.bb.razaInaltime = collider.dimensiune1;
            return;
        }

        float cos_phi =  abs(cos(phi));
        float sin_phi =  abs(sin(phi));

        float w = collider.dimensiune1 / 2.0f;
        float h = collider.dimensiune2 / 2.0f;

        collider.bb.razaLatime = w *cos_phi + h * sin_phi;
        collider.bb.razaInaltime = w*sin_phi + h*cos_phi;

    }
    
    
    // Creeaza o Bara (Dreptunghiulara)
    static rigid Bara( float x, float y, float Lungime, float Grosime, float Masa) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.collider.dimensiune1 = Lungime; 
        r.collider.dimensiune2 = Grosime;
        r.M = Masa;
        r.J = (Masa * (Lungime * Lungime + Grosime * Grosime)) / 12.0f;
        r.collider.tip = DREPTUNGHI;
        return r;
    }

    static rigid Disc( float x, float y, float Raza, float Masa) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.collider.dimensiune1 = Raza;
        r.collider.dimensiune2 = Raza;
        r.M = Masa;
        r.J = (Masa * Raza * Raza) / 2.0f;
        r.collider.tip = CERC;
        return r;
    }

    // Creeaza un punct fix (Lumea)
    static rigid Fix( float x, float y) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.M = 1e12f; // Masa infinita
        r.J = 1e12f;
        r.collider.dimensiune1 = 0.0f;
        r.collider.dimensiune2 = 0.0f;
        r.collider.tip = PUNCT;
        return r;
    }
};
