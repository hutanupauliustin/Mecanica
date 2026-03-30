#include "cmath"
#include "rigid.h"
#include <cstring>

tipMaterial obtineMaterialDupaNume(const char* nume) {
    if (std::strcmp(nume, "Cauciuc") == 0) return materiale::Cauciuc;
    if (std::strcmp(nume, "Gheata") == 0)  return materiale::Gheata;
    if (std::strcmp(nume, "Piatra") == 0)  return materiale::Piatra;
    
    return materiale::Lemn;     //default
}

rigid::rigid() : pozitie(0.0f, 0.0f), phi(0.0f), viteza(0.0f, 0.0f), omega(0.0f), M(1.0f), J(1.0f) {
        collider.tip = PUNCT;
        collider.dimensiune1 = 1.0f;
        collider.dimensiune2 = 1.0f;
        collider.bb.razaInaltime = 1.0f;
        collider.bb.razaLatime = 1.0f;

        material = materiale::Lemn;
    }

    rigid::rigid (float x_initial, float y_initial, float phi_initial, float masa, float momentInertie)
        : pozitie(x_initial, y_initial), phi(phi_initial), M(masa), J(momentInertie), viteza(0.0f, 0.0f), omega(0.0f) {

            material = materiale::Lemn;
        }
        
    void rigid::adauagaForte(float modul_forta, float x_aplicare, float y_aplicare, float u_x, float u_y ){
        fortaExterna F;
        F.modul = modul_forta;
        F.u = vec2(u_x, u_y);
        F.punct_aplicatie = vec2(x_aplicare, y_aplicare);
        forte.push_back(F);
    }

    vec2 rigid::localToGlobal(vec2 punctLocal) {
        float cos_phi = std::cos(phi);
        float sin_phi = std::sin(phi);
        
        vec2 punctRotit(
            punctLocal.x * cos_phi - punctLocal.y * sin_phi,
            punctLocal.x * sin_phi + punctLocal.y * cos_phi
        );
        
        return pozitie + punctRotit;
    }

    vec2 rigid::vitezaAbsolutaPunct(vec2 punctLocal) {
        vec2 v_rotatie(
            -punctLocal.y * omega,
             punctLocal.x * omega
        );
        return viteza + v_rotatie;
    }

    vec2 rigid::globalToLocal(vec2 punctGlobal) {
        vec2 d = punctGlobal - pozitie;
        float cos_phi = std::cos(phi);
        float sin_phi = std::sin(phi);
        
        return vec2(
            d.x * cos_phi + d.y * sin_phi,
            -d.x * sin_phi + d.y * cos_phi
        );
    }

    void rigid::aflaForteProprii(float g)
    {
        tau.forta.x = 0;
        tau.forta.y = 0;
        tau.moment = 0;

        // Daca masa este foarte mare (infinita), consideram corpul fix si nu ii aplicam greutate
        // pentru a evita erori matematice (Infinity * ceva = NaN)
        if(M > 1e10f) {
            ;
        } else {
            
            fortaVizuala forta_gravitatie;
            forta_gravitatie.valoare.x = 0;
            forta_gravitatie.valoare.y = (-1)*M*g;
            forta_gravitatie.tip = FORTA_GREUTATE;
            forta_gravitatie.punct_aplicare = pozitie;
            forte_desen.forte.push_back(forta_gravitatie); 

            fortaExterna gravitatie;
            gravitatie.u.x = 0;
            gravitatie.u.y = 1;
            gravitatie.modul =  (-1)*M*g;
            gravitatie.punct_aplicatie = this->pozitie;
            this->forte.push_back(gravitatie);

            float drag = collider.coeficientAerodinamic;
            tau.forta.x -= drag * viteza.x;
            tau.forta.y -= drag * viteza.y;
            tau.moment -= drag * omega * 0.5f;
        }
    }

    void rigid::reducereTorsor(){
        
        for(int i = 0; i < this->forte.size(); i++){
            fortaExterna F = this->forte[i];
            vec2 vector_forta = F.u * F.modul;
            vec2 r = F.punct_aplicatie - pozitie;

            this->tau.forta += vector_forta;
            this->tau.moment += r.vectorial(vector_forta);
        }

        forte.clear();
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
    
    
    rigid rigid::Bara( float x, float y, float Lungime, float Grosime, float Masa, const char* numeMaterial) {
        rigid r;
        r.index = 0;
        r.pozitie = vec2(x, y);
        r.collider.dimensiune1 = Lungime; 
        r.collider.dimensiune2 = Grosime;
        r.M = Masa;
        r.J = (Masa * (Lungime * Lungime + Grosime * Grosime)) / 12.0f;
        r.collider.tip = DREPTUNGHI;
        r.collider.coeficientAerodinamic = ((Lungime + Grosime) / 2.0f) * 1.05f;

        r.material = obtineMaterialDupaNume(numeMaterial);

        return r;
    }

    rigid rigid::Disc( float x, float y, float Raza, float Masa, const char* numeMaterial) {
        rigid r;
        r.index = 0;
        r.pozitie = vec2(x, y);
        r.collider.dimensiune1 = Raza;
        r.collider.dimensiune2 = Raza;
        r.M = Masa;
        r.J = (Masa * Raza * Raza) / 2.0f;
        r.collider.tip = CERC;
        r.collider.coeficientAerodinamic = Raza * 0.47f;

        r.material = obtineMaterialDupaNume(numeMaterial);  
        return r;
    }

    // Creeaza un punct fix (Lumea)
    rigid rigid::Fix( float x, float y) {
        rigid r;
        r.index = 0;
        r.pozitie = vec2(x, y);
        r.M = 1e12f; // Masa infinita
        r.J = 1e12f;
        r.collider.dimensiune1 = 0.0f;
        r.collider.dimensiune2 = 0.0f;
        r.collider.tip = PUNCT;

        r.material = materiale::Lemn;
        return r;
    }