#pragma once
#include "sistem.h"
#include <vector>

class editor;
struct fantomaUI;

class InstrumentEditor{
public:
    virtual ~InstrumentEditor() = default;

    virtual void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) = 0;
    virtual void clickDreapta(sistem& S, editor& E) = 0;

    virtual void anuleaza(editor& E) = 0;

    virtual void miscareMouse(sistem& S, editor& E, float mouse_x, float mouse_y) {}
    virtual void eliberareClickStanga(sistem& S, editor& E) {}

    virtual void pregatesteFantome(std::vector<fantomaUI> &elementeUI, float mouse_x, float mouse_y, sistem& S) {}

};

class InstrumentAdaugaArticulatie : public InstrumentEditor{
public:

    int pas =0;
    int id_corp_A = -1;
    vec2 punct_A_local;
    float unghi_fantoma =0.0f;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            id_corp_A = E.gasesteCorpSubMouse(S);
            if (id_corp_A != -1) {
                punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
                unghi_fantoma = S.corpuri[id_corp_A].phi;
                pas = 1;
            }
        } else if (pas == 1){
            int id_corp_B = E.gasesteCorpSubMouse(S);
            if (id_corp_B == -1) id_corp_B = 0; // fundalul

            if (id_corp_B != id_corp_A) {
                rigid& corpA = S.corpuri[id_corp_A];
                
                if (id_corp_A != 0) {
                    corpA.phi = unghi_fantoma;
                    // Aici poți adăuga logica de mutare la poziția mouse-ului
                    corpA.viteza = vec2(0.0f, 0.0f); 
                    corpA.omega = 0.0f;
                }

                S.adaugaLegaturi(articulatie::Creaza(corpA, S.corpuri[id_corp_B], mouse_x, mouse_y));
                S.actualizeazaMatriceFizica();
                
                E.schimbaInstrumentCurent(new InstrumentSelectie());
            }
        }
    }

    void clickDreapta(sistem& S, editor& E) override {
        if (pas == 1) {
            pas = 0;
            id_corp_A = -1;
        } else {
            anuleaza(E);
        }
    }

    void anuleaza(editor& E) override {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }
    
    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        if (elementeUI.size() < 2) elementeUI.resize(2);
        
        auto& fantoma_corp = elementeUI[0];
        auto& fantoma_leg = elementeUI[1];

        fantoma_corp.activa = false;
        fantoma_leg.activa = false;

        if (pas == 1) {
            if (id_corp_A != 0) {
                rigid& corpA = S.corpuri[id_corp_A];
                fantoma_corp.tip = corpA.collider.tip;
                fantoma_corp.dim1 = corpA.collider.dimensiune1;
                fantoma_corp.dim2 = corpA.collider.dimensiune2;
                fantoma_corp.col = { corpA.collider.culoare.r, corpA.collider.culoare.g, corpA.collider.culoare.b, 0.5f };
                fantoma_corp.phi = unghi_fantoma;
                fantoma_corp.activa = true;
                
                vec2 offset_rotit(
                    punct_A_local.x * cos(unghi_fantoma) - punct_A_local.y * sin(unghi_fantoma),
                    punct_A_local.x * sin(unghi_fantoma) + punct_A_local.y * cos(unghi_fantoma)
                );
                fantoma_corp.x = mouse_x - offset_rotit.x;
                fantoma_corp.y = mouse_y - offset_rotit.y;
            }
            
            fantoma_leg.activa = true;
            fantoma_leg.x = mouse_x;
            fantoma_leg.y = mouse_y;
            fantoma_leg.dim1 = 0.5f;
            fantoma_leg.dim2 = 0.5f;
            fantoma_leg.phi = (id_corp_A != 0) ? unghi_fantoma : 0.0f;
            fantoma_leg.tip = 1; // Vizual specific pentru Articulatie
            fantoma_leg.col = {1.0f, 1.0f, 1.0f, 0.8f};
        }
    }

};

class InstrumentAdaugareIncastrare : public InstrumentEditor {
private:
    int pas = 0;      
    int id_corp_A = -1;
    vec2 punct_A_local;
    float unghi_fantoma = 0.0f; 

public:
    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            id_corp_A = E.gasesteCorpSubMouse(S);
            if (id_corp_A != -1) {
                punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
                unghi_fantoma = S.corpuri[id_corp_A].phi;
                pas = 1;
            }
        } 
        else if (pas == 1) {
            int id_corp_B = E.gasesteCorpSubMouse(S);
            if (id_corp_B == -1) id_corp_B = 0; 

            if (id_corp_B != id_corp_A) {
                rigid& corpA = S.corpuri[id_corp_A];
                
                if (id_corp_A != 0) {
                    corpA.phi = unghi_fantoma;
                    // Aici poți adăuga logica de mutare la poziția mouse-ului
                    corpA.viteza = vec2(0.0f, 0.0f); 
                    corpA.omega = 0.0f;
                }

                S.adaugaLegaturi(incastrare::Creaza(corpA, S.corpuri[id_corp_B], mouse_x, mouse_y));
                S.actualizeazaMatriceFizica();
                
                E.schimbaInstrumentCurent(new InstrumentSelectie());
            }
        }
    }

    void clickDreapta(sistem& S, editor& E) override {
        if (pas == 1) {
            pas = 0;
            id_corp_A = -1;
        } else {
            anuleaza(E);
        }
    }

    void anuleaza(editor& E) override {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        if (elementeUI.size() < 2) elementeUI.resize(2);
        
        auto& fantoma_corp = elementeUI[0];
        auto& fantoma_leg = elementeUI[1];

        fantoma_corp.activa = false;
        fantoma_leg.activa = false;

        if (pas == 1) {
            if (id_corp_A != 0) {
                rigid& corpA = S.corpuri[id_corp_A];
                fantoma_corp.tip = corpA.collider.tip;
                fantoma_corp.dim1 = corpA.collider.dimensiune1;
                fantoma_corp.dim2 = corpA.collider.dimensiune2;
                fantoma_corp.col = { corpA.collider.culoare.r, corpA.collider.culoare.g, corpA.collider.culoare.b, 0.5f };
                fantoma_corp.phi = unghi_fantoma;
                fantoma_corp.activa = true;
                
                vec2 offset_rotit(
                    punct_A_local.x * cos(unghi_fantoma) - punct_A_local.y * sin(unghi_fantoma),
                    punct_A_local.x * sin(unghi_fantoma) + punct_A_local.y * cos(unghi_fantoma)
                );
                fantoma_corp.x = mouse_x - offset_rotit.x;
                fantoma_corp.y = mouse_y - offset_rotit.y;
            }
            
            fantoma_leg.activa = true;
            fantoma_leg.x = mouse_x;
            fantoma_leg.y = mouse_y;
            fantoma_leg.dim1 = 0.5f;
            fantoma_leg.dim2 = 0.5f;
            fantoma_leg.phi = (id_corp_A != 0) ? unghi_fantoma : 0.0f;
            fantoma_leg.tip = 2; // Vizual specific pentru Incastrare
            fantoma_leg.col = {1.0f, 1.0f, 1.0f, 0.8f};
        }
    }
};

class InstrumentAdaugaArc : public InstrumentEditor {
public:
    int pas = 0;
    int id_corp_A = -1;
    vec2 punct_A_local;

    float arc_k = 100.0f;
    float arc_d = 5.0f;
    float arc_l0_procent = 100.0f;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            id_corp_A = E.gasesteCorpSubMouse(S);
            if (id_corp_A == -1) id_corp_A = 0; // Poate fi prins si de "Lume" (fundal)

            punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
            pas = 1;
        } else if (pas == 1) {
            int id_corp_B = E.gasesteCorpSubMouse(S);
            if (id_corp_B == -1) id_corp_B = 0;

            vec2 punct_B_local = S.corpuri[id_corp_B].globalToLocal(vec2(mouse_x, mouse_y));
            
            vec2 pA_global = S.corpuri[id_corp_A].localToGlobal(punct_A_local);
            vec2 pB_global = S.corpuri[id_corp_B].localToGlobal(punct_B_local);
            
            float dist = (pA_global - pB_global).modul();
            float l0_real = dist * (arc_l0_procent / 100.0f);
            
            arc arc_nou = arc::Creaza(S.corpuri[id_corp_A], S.corpuri[id_corp_B],
                                      pA_global.x, pA_global.y, pB_global.x, pB_global.y,
                                      arc_k, arc_d, l0_real);
            S.adaugaGeneratorForte(new arc(arc_nou));
            S.actualizeazaMatriceFizica();

            id_arc_curent = S.arcuri.size() - 1;
        } else if (pas == 2){
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }

    void clickDreapta(sistem& S, editor& E) override {
        if (pas == 1) {
            pas = 0;
            id_corp_A = -1;
        } else if (pas == 2) {
            S.arcuri.pop_back();
            E.schimbaInstrumentCurent(new InstrumentSelectie());

        } else {
            anuleaza(E);
        }
    }

    void anuleaza(editor& E) override {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        if (elementeUI.size() < 2) elementeUI.resize(2);
        elementeUI[0].activa = false;
        elementeUI[1].activa = false;

        if (pas == 1) {
            elementeUI[1].activa = true;
            vec2 pA_global = S.corpuri[id_corp_A].localToGlobal(punct_A_local);
            vec2 p_mouse(mouse_x, mouse_y);
            vec2 diferenta = p_mouse - pA_global;
            
            elementeUI[1].phi = std::atan2(diferenta.y, diferenta.x);
            elementeUI[1].dim1 = diferenta.modul(); 
            elementeUI[1].dim2 = elementeUI[1].dim1;  
            elementeUI[1].x = (pA_global.x + p_mouse.x) / 2.0f; 
            elementeUI[1].y = (pA_global.y + p_mouse.y) / 2.0f;
            elementeUI[1].tip = 4; 
            elementeUI[1].col = {0.5f, 1.0f, 0.5f, 0.8f};
        }
    }
};

class InstrumentSelectie : public InstrumentEditor {
private:
    bool se_muta = false;
    int id_corp_mutat = -1;
    float offset_x = 0.0f;
    float offset_y = 0.0f;

public:
    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        int id = E.gasesteCorpSubMouse(S);
        
        if (id != -1) {
            // Am dat click pe un corp -> Il selectam
            E.corpuriSelectate.clear(); // Curatam selectia veche
            E.corpuriSelectate.push_back(id);
            
            // Incepem logica de Drag & Drop
            se_muta = true;
            id_corp_mutat = id;
            
            // Calculam de unde am apucat corpul fata de centrul lui
            offset_x = S.corpuri[id].pozitie.x - mouse_x;
            offset_y = S.corpuri[id].pozitie.y - mouse_y;
            
            // Îl "înghețăm" din mișcare cât timp ținem mâna pe el
            S.corpuri[id].viteza = vec2(0.0f, 0.0f);
            S.corpuri[id].omega = 0.0f;
        } else {
            // Am dat click in gol -> Deselectam tot
            E.corpuriSelectate.clear();
        }
    }

    void miscareMouse(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        // Daca tinem click stanga apasat pe un corp si miscam mouse-ul
        if (se_muta && id_corp_mutat != -1 && id_corp_mutat != 0) {
            S.corpuri[id_corp_mutat].pozitie.x = mouse_x + offset_x;
            S.corpuri[id_corp_mutat].pozitie.y = mouse_y + offset_y;
            S.corpuri[id_corp_mutat].viteza = vec2(0.0f, 0.0f);
            
            S.actualizeazaMatriceFizica();
        }
    }

    void eliberareClickStanga(sistem& S, editor& E) override {
        // Cand luam degetul de pe mouse, oprim "tragerea"
        se_muta = false;
        id_corp_mutat = -1;
    }

    void clickDreapta(sistem& S, editor& E) override {
        anuleaza(E);
    }

    void anuleaza(editor& E) override {
        E.corpuriSelectate.clear();
        se_muta = false;
        id_corp_mutat = -1;
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        // Cat timp suntem in modul selectie, ascundem orice fantoma din UI
        for(auto& f : elementeUI) {
            f.activa = false;
        }
    }
};