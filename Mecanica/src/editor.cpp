#include "editor.h"
#include <cmath>
#include <algorithm>

editor::editor(){
    mod_curent = MOD_RULARE;
    mod_vizualizare = 0;
    cadru_activ = 0;
    mouse_x = 0.0f;
    mouse_y = 0.0f;
    legatura_corpA = -1;
    stare_legatura = 0;

    corpApasat = -1;

    elementeUI.resize(2);   //pentru fantoma de corp si de legatura
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
        if (this->mod_curent == MOD_PLASARE_LEGATURA_1 && this->legatura_corpA == k) continue;

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
