#pragma once
#include <cmath>

class rigid
{
public:
    int index = 0; // Indexul corpului in vectorul sistemului
    float x, y, phi;
    float v_x, v_y, omega; // coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
    float M, J;
    float f_x, f_y, moment;
    float w, h; // Latime si Inaltime (pentru desenare)
    int tip;    // 0 = Invizibil, 1 = Cerc, 2 = Dreptunghi, 3 = Triunghi/Fix

    rigid() :  x(0), y(0), phi(0), v_x(0), v_y(0), omega(0), M(1), J(1), f_x(0), f_y(0), moment(0), w(1), h(1), tip(0) {}

    rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie)
        :  x(x_initial), y(y_initial), phi(phi_initial), M(masa), J(momentInertie), v_x(0), v_y(0), omega(0), f_x(0), f_y(0), moment(0), w(1), h(1), tip(0) {}

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
    // Creeaza o Bara (Dreptunghiulara)
    static rigid Bara( float x, float y, float Lungime, float Grosime, float Masa) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.w = Lungime; r.h = Grosime;
        r.M = Masa;
        r.J = (Masa * (Lungime * Lungime + Grosime * Grosime)) / 12.0f;
        r.tip = 2; // Dreptunghi
        return r;
    }

    static rigid Disc( float x, float y, float Raza, float Masa) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.w = Raza * 2.0f; // Width = Diametru
        r.h = Raza * 2.0f; // Height = Diametru
        r.M = Masa;
        r.J = (Masa * Raza * Raza) / 2.0f;
        r.tip = 1; // Cerc
        return r;
    }

    // Creeaza un punct fix (Lumea)
    static rigid Fix( float x, float y) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.M = 1e12f; // Masa infinita
        r.J = 1e12f;
        r.w = 1.0f; r.h = 1.0f;
        r.tip = 0; // Invizibil
        return r;
    }
};
