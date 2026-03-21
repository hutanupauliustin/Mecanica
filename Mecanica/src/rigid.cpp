#include "cmath"
#include "rigid.h"
    
    rigid::rigid() :  x(0), y(0), phi(0), v_x(0), v_y(0), omega(0), M(1), J(1), f_x(0), f_y(0){
        collider.tip = PUNCT;
        collider.dimensiune1 = 1.0f;
        collider.dimensiune2 = 1.0f;
        collider.bb.razaInaltime = 1.0f;
        collider.bb.razaLatime = 1.0f;
    }

    rigid::rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie)
        :  x(x_initial), y(y_initial), phi(phi_initial), M(masa), J(momentInertie), v_x(0), v_y(0), omega(0), f_x(0), f_y(0), moment(0){}

    void rigid::aflaForteProprii(float g)
    {
        // Daca masa este foarte mare (infinita), consideram corpul fix si nu ii aplicam greutate
        // pentru a evita erori matematice (Infinity * ceva = NaN)
        if(M > 1e10f) {
            f_x = 0;
            f_y = 0;
            moment =0;
        } else {
            f_x = 0;
            f_y = -g * M; 

            float drag = collider.coeficientAerodinamic;
            f_x -= drag * v_x;
            f_y -= drag * v_y;
            moment -= drag * omega * 0.5f;
        }
    }

    void rigid::coordPunctPeCorp(float &punct_x, float &punct_y, float d_x, float d_y){                //d_x si d_y sunt coordonatele punctului fata de sistemul de referinta mobil al corpului
        punct_x = x + d_x* cos(phi) - d_y * sin(phi);
        punct_y = y + d_x* sin(phi) + d_y * cos(phi);
    }

    void rigid::vitezaPunctPeCorp(float &punct_v_x, float &punct_v_y, float d_x, float d_y){
        punct_v_x = v_x  - d_y * omega;
        punct_v_y = v_y  - d_x * omega;
    }
    
    void rigid::seteazaBoundingBox(){                  // "deseneaza" o cutie dreptunghiulara cu laturile paralele cu axele OX si OY ale sistemului, care sa cuprinda intreg rigidul
        
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
    
    
    rigid rigid::Bara( float x, float y, float Lungime, float Grosime, float Masa) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.collider.dimensiune1 = Lungime; 
        r.collider.dimensiune2 = Grosime;
        r.M = Masa;
        r.J = (Masa * (Lungime * Lungime + Grosime * Grosime)) / 12.0f;
        r.collider.tip = DREPTUNGHI;
        r.collider.coeficientAerodinamic = ((Lungime + Grosime) / 2.0f) * 1.05f;
        return r;
    }

    rigid rigid::Disc( float x, float y, float Raza, float Masa) {
        rigid r;
        r.index = 0;
        r.x = x; r.y = y;
        r.collider.dimensiune1 = Raza;
        r.collider.dimensiune2 = Raza;
        r.M = Masa;
        r.J = (Masa * Raza * Raza) / 2.0f;
        r.collider.tip = CERC;
        r.collider.coeficientAerodinamic = Raza * 0.47f;
        return r;
    }

    // Creeaza un punct fix (Lumea)
    rigid rigid::Fix( float x, float y) {
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