#include "editor.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

editor::editor(){
    mod_curent = MOD_RULARE;
    mod_vizualizare = 0;
    cadru_activ = 0;
    mouse_x = 0.0f;
    mouse_y = 0.0f;
    frameCount = 0;

    dt = 0.001f;
    t = 0.0f;

    scala_timp = 1.0f;
    corpuriSelectate.resize(0);

    elementeUI.resize(2);   //pentru fantoma de corp si de legatura

    valoriSimulate.reserve(16000);
    valoriSimulate.reserve(30); // 1000 de corpuri pre-alocate sunt arhisuficiente

    fisier_export.open(nume_fisier_export);
    if (fisier_export.is_open()) {
        fisier_export << "Timp,ID_Corp,PozitieX,PozitieY,UnghiPhi,VitezaX,VitezaY,VitezaOmega,AcceleratieX,AcceleratieY,AcceleratieEpsilon\n";
    }
}

editor::~editor() {
    if (fisier_export.is_open()) {
        fisier_export.close();
    }
    std::remove(nume_fisier_export.c_str());
}

void editor::mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY){   

    S.corpuri[idCorp].pozitie.x = this->mouse_x + offsetX;
    S.corpuri[idCorp].pozitie.y = this->mouse_y + offsetY;

}

int editor::gasesteCorpSubMouse(sistem &S){
    int celMaiAproape_id = -1;
    float min_distanta = 999999.9f;

    for (int k = S.corpuri.size() - 1; k >= 1; k--) { 
        if (!S.corpuri[k].activ) continue;
        if (S.corpuri[k].collider.obiectVirtual) continue; // Nu vrem să selectăm fantomele
        
        // Ignorăm corpul A deja selectat, pentru a putea găsi corpul B aflat sub el
        if (this->mod_curent == MOD_ADAUGARE_LEGATURA_PAS_2 && this->adaugare_corp_A == k) continue;

        // În modul Editare, putem impune să selectăm doar corpurile din layerul activ
        if (this->mod_curent == MOD_EDITARE && S.corpuri[k].collider.cadru != this->cadru_activ) continue;

        rigid &target = S.corpuri[k];
        
        float dx = this->mouse_x - target.pozitie.x;
        float dy = this->mouse_y - target.pozitie.y;
        
       vec2 punct_local = target.globalToLocal(vec2(this->mouse_x,this->mouse_y));

        bool lovit = false;
        float cur_dist = 999999.0f;

        if (target.collider.tip == DREPTUNGHI) {
            float hw = target.collider.dimensiune1 / 2.0f;
            float hh = target.collider.dimensiune2 / 2.0f;
            // Adăugăm un halo subtil doar în modul editare
            float halo = (this->mod_curent == 1) ? std::max(0.15f, std::min(hw, hh) * 0.5f) : 0.0f;

            if (std::abs(punct_local.x) <= hw + halo && std::abs(punct_local.y) <= hh + halo) {
                lovit = true;
                cur_dist = std::sqrt(dx*dx + dy*dy);
            }
        } else if (target.collider.tip == CERC) {
            float R = target.collider.dimensiune1;
            float halo = (this->mod_curent == 1) ? std::max(0.15f, R * 0.3f) : 0.0f;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist <= R + halo) {
                lovit = true;
                cur_dist = dist;
            }
        }

        if (lovit) {
            if (celMaiAproape_id == -1 || cur_dist < min_distanta - 0.1f) {
                min_distanta = cur_dist;
                celMaiAproape_id = k;
            }
        }
    }

    return celMaiAproape_id;
}

void editor::sincronizeazaMemorie(sistem &S){
    if (valoriSimulate.size() < S.corpuri.size()) {
        size_t old_size = valoriSimulate.size();
        valoriSimulate.resize(S.corpuri.size());
        
        // Pre-alocăm capacitatea maximă exactă pentru istoricul corpurilor noi
        for (size_t i = old_size; i < valoriSimulate.size(); i++) {
            valoriSimulate[i].timpAfisat.reserve(valoriSimulate[i].capacitate_maxima);
            for (int axa = 0; axa < TOTAL_PARAMETRII; axa++) {
                valoriSimulate[i].axe[axa].reserve(valoriSimulate[i].capacitate_maxima);
            }
        }
    }
    
    // Curățăm istoricul corpurilor inactive pentru a preveni amestecarea graficelor dacă indexul este refolosit
    for(size_t i = 0; i < S.corpuri.size(); i++){
        if (!S.corpuri[i].activ && valoriSimulate.size() > i) {
            
            valoriSimulate[i].timpAfisat.clear();
            for (int axa = 0; axa < TOTAL_PARAMETRII; axa++) {
                valoriSimulate[i].axe[axa].clear();
            }
            valoriSimulate[i].offset = 0;
        }
    }
}

void editor::incarcaDatePentruGrafic(sistem &S){
    std::string csv_buffer;
    csv_buffer.reserve(S.corpuri.size() * 100);
    
    for(size_t i = 0; i < S.corpuri.size(); i++){

        if (!S.corpuri[i].activ || S.corpuri[i].M > 1e10f) continue;
        
        IstoricCorp& istoric = valoriSimulate[i];

        if (istoric.timpAfisat.size() < istoric.capacitate_maxima) {
            istoric.timpAfisat.push_back(this->t);
            istoric.axe[POZITIE_X].push_back(S.corpuri[i].pozitie.x);
            istoric.axe[POZITIE_Y].push_back(S.corpuri[i].pozitie.y);
            istoric.axe[POZITIE_PHI].push_back(S.corpuri[i].phi);
            istoric.axe[VITEZA_X].push_back(S.corpuri[i].viteza.x);
            istoric.axe[VITEZA_Y].push_back(S.corpuri[i].viteza.y);
            istoric.axe[VITEZA_OMEGA].push_back(S.corpuri[i].omega);
            istoric.axe[ACCELERATIE_X].push_back(S.corpuri[i].forte_desen.acc_cadru.x);
            istoric.axe[ACCELERATIE_Y].push_back(S.corpuri[i].forte_desen.acc_cadru.y);
            istoric.axe[ACCELERATIE_EPSILON].push_back(S.corpuri[i].forte_desen.eps_cadru);
        } else {
            istoric.timpAfisat[istoric.offset] = this->t;
            istoric.axe[POZITIE_X][istoric.offset] = S.corpuri[i].pozitie.x;
            istoric.axe[POZITIE_Y][istoric.offset] = S.corpuri[i].pozitie.y;
            istoric.axe[POZITIE_PHI][istoric.offset] = S.corpuri[i].phi;
            istoric.axe[VITEZA_X][istoric.offset] = S.corpuri[i].viteza.x;
            istoric.axe[VITEZA_Y][istoric.offset] = S.corpuri[i].viteza.y;
            istoric.axe[VITEZA_OMEGA][istoric.offset] = S.corpuri[i].omega;
            istoric.axe[ACCELERATIE_X][istoric.offset] = S.corpuri[i].forte_desen.acc_cadru.x;
            istoric.axe[ACCELERATIE_Y][istoric.offset] = S.corpuri[i].forte_desen.acc_cadru.y;
            istoric.axe[ACCELERATIE_EPSILON][istoric.offset] = S.corpuri[i].forte_desen.eps_cadru;
            
            istoric.offset = (istoric.offset + 1) % istoric.capacitate_maxima;
        }
        
        char linie_csv[256];
        std::snprintf(linie_csv, sizeof(linie_csv), "%.4f,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                      this->t, i, S.corpuri[i].pozitie.x, S.corpuri[i].pozitie.y, S.corpuri[i].phi,
                      S.corpuri[i].viteza.x, S.corpuri[i].viteza.y, S.corpuri[i].omega,
                      S.corpuri[i].forte_desen.acc_cadru.x, S.corpuri[i].forte_desen.acc_cadru.y, S.corpuri[i].forte_desen.eps_cadru);
        csv_buffer += linie_csv;
    }
    
    if (fisier_export.is_open() && !csv_buffer.empty()) {
        fisier_export << csv_buffer;
    }
}
