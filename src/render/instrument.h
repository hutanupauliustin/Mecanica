#pragma once
#include "sistem.h"
#include "editor.h"
#include <vector>
#include "imgui.h"

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

class InstrumentSelectie : public InstrumentEditor {
public:

    std::vector<std::pair<int, vec2>> offseturi_drag;

    bool mouse_apasat = false;
    vec2 pozitie_click_initial;
    int id_corp_la_click = -1;
    bool in_drag = false;
    static constexpr float PRAG_DRAG = 0.2f;

    void actualizeazaSelectatVizual(sistem& S, editor& E) {

        for (auto& corp : S.corpuri) {
        corp.collider.selectat = 0;
    }
    for (int id : E.corpuriSelectate) {
        if (id >= 0 && id < (int)S.corpuri.size()) {
            S.corpuri[id].collider.selectat = 1;
        }
    }
    }

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        
        if (ImGui::GetIO().WantCaptureMouse) return;

        mouse_apasat = true;
        in_drag = false;
        offseturi_drag.clear();
        pozitie_click_initial = vec2(mouse_x, mouse_y);

        id_corp_la_click = -1;
        for (int i = 1; i < (int)S.corpuri.size(); i++) {
            if (S.corpuri[i].collider.subMouse) {
                id_corp_la_click = i;
                break;
            }
        }

    }

    void miscareMouse(sistem& S, editor& E, float mouse_x, float mouse_y) override {
    if (!mouse_apasat) return;
    
    vec2 delta = vec2(mouse_x, mouse_y) - pozitie_click_initial;
    
    if (!in_drag && delta.modul() > PRAG_DRAG) {

        in_drag = true;

        if (id_corp_la_click != -1 && id_corp_la_click != 0) {
            if (std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), 
                          id_corp_la_click) == E.corpuriSelectate.end()) {
                E.corpuriSelectate.clear();
                E.corpuriSelectate.push_back(id_corp_la_click);
                actualizeazaSelectatVizual(S, E);
            }
        
            for (int id : E.corpuriSelectate) {
                vec2 offset(
                    S.corpuri[id].pozitie.x - mouse_x,
                    S.corpuri[id].pozitie.y - mouse_y
                );
                offseturi_drag.push_back({id, offset});
                }
            }
    }
    
    if (in_drag) {
    for (auto& [id, offset] : offseturi_drag) {
        S.corpuri[id].pozitie.x = mouse_x + offset.x;
        S.corpuri[id].pozitie.y = mouse_y + offset.y;
        S.corpuri[id].viteza = vec2(0.0f, 0.0f);
    }

    S.actualizeazaMatriceFizica();
    }
}

   void eliberareClickStanga(sistem& S, editor& E) override {
    if (!in_drag && id_corp_la_click != -1) {
        bool shift_apasat = glfwGetKey(E.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        if (!shift_apasat) {
            E.corpuriSelectate.clear();
        }
       
        auto& sel = E.corpuriSelectate;
        if (std::find(sel.begin(), sel.end(), id_corp_la_click) == sel.end()) {
            sel.push_back(id_corp_la_click);
        }
    } else if (!in_drag) {
        E.corpuriSelectate.clear();
    }

    actualizeazaSelectatVizual(S,E);

    mouse_apasat = false;
    in_drag = false;
    id_corp_la_click = -1;
}

    void clickDreapta(sistem& S, editor& E) override {
        anuleaza(E);
    }

    void anuleaza(editor& E) override {
        E.corpuriSelectate.clear();
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        // Cat timp suntem in modul selectie, ascundem orice fantoma din UI
        for(auto& f : elementeUI) {
            f.activa = false;
        }
    }
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
                    vec2 offset_global = corpA.localToGlobal(punct_A_local) - corpA.pozitie;

                    corpA.pozitie = vec2(mouse_x, mouse_y) - offset_global;
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

class InstrumentAdaugaIncastrare : public InstrumentEditor {
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
                    vec2 offset_global = corpA.localToGlobal(punct_A_local) - corpA.pozitie;

                    corpA.pozitie = vec2(mouse_x, mouse_y) - offset_global;
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

    generatorForte* arc_adaugat = nullptr;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            id_corp_A = E.gasesteCorpSubMouse(S);
            if (id_corp_A == -1) id_corp_A = 0; 

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
            
            arc* arc_nou = arc::Creaza(S.corpuri[id_corp_A], S.corpuri[id_corp_B],
                                      pA_global.x, pA_global.y, pB_global.x, pB_global.y,
                                      arc_k, arc_d, l0_real);
            S.adaugaGeneratorForte(arc_nou);
            S.actualizeazaMatriceFizica();
            
            arc_adaugat = arc_nou;
            pas = 2;
        } else if (pas == 2){
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }

    void clickDreapta(sistem& S, editor& E) override {
        if (pas == 1) {
            pas = 0;
            id_corp_A = -1;
        } else if (pas == 2) {
            if (arc_adaugat) arc_adaugat->activ = 0;
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

class InstrumentAdaugaFir : public InstrumentEditor {
public:
    int pas = 0;
    int id_corp_A = -1;
    vec2 punct_A_local;

    legatura* fir_adaugat = nullptr;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            id_corp_A = E.gasesteCorpSubMouse(S);
            if (id_corp_A == -1) id_corp_A = 0; 

            punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
            pas = 1;
        } else if (pas == 1) {
            int id_corp_B = E.gasesteCorpSubMouse(S);
            if (id_corp_B == -1) id_corp_B = 0;

            vec2 punct_B_local = S.corpuri[id_corp_B].globalToLocal(vec2(mouse_x, mouse_y));
            
            vec2 pA_global = S.corpuri[id_corp_A].localToGlobal(punct_A_local);
            vec2 pB_global = S.corpuri[id_corp_B].localToGlobal(punct_B_local);
            
            float dist = (pA_global - pB_global).modul();
            
            fir* fir_nou = fir::Creaza(S.corpuri[id_corp_A], S.corpuri[id_corp_B],
                                      pA_global.x, pA_global.y, pB_global.x, pB_global.y);
            S.adaugaLegaturi(fir_nou);
            S.actualizeazaMatriceFizica();
            
            fir_adaugat = fir_nou;
            pas = 2;
        } else if (pas == 2){
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }

    void clickDreapta(sistem& S, editor& E) override {
        if (pas == 1) {
            pas = 0;
            id_corp_A = -1;
        } else if (pas == 2) {
            if (fir_adaugat) fir_adaugat->activ = 0;
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
            elementeUI[1].dim2 = elementeUI[1].dim1 / 25;  
            elementeUI[1].x = (pA_global.x + p_mouse.x) / 2.0f; 
            elementeUI[1].y = (pA_global.y + p_mouse.y) / 2.0f;
            elementeUI[1].tip = 2; 
            elementeUI[1].col = {1.0f, 1.0f, 1.0f, 0.8f};
        }
    }
};

class InstrumentAdaugaCorp : public InstrumentEditor {
public:
    int tip_corp = DREPTUNGHI;      // CERC sau DREPTUNGHI
    float dimensiune1 = 2.0f;       // Lungime (bara) sau Raza (disc)
    float dimensiune2 = 0.4f;       // Grosime (bara) — ignorat pt disc
    float masa = 1.0f;
    culoare col = {0.4f, 0.8f, 0.4f, 1.0f};
    const char* material = "Lemn";

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        rigid corp_nou;
        if (tip_corp == CERC) {
            corp_nou = rigid::Disc(mouse_x, mouse_y, dimensiune1, masa, material);
        } else {
            corp_nou = rigid::Bara(mouse_x, mouse_y, dimensiune1, dimensiune2, masa, material);
        }
        corp_nou.collider.culoare = col;
        S.adaugaCorpuri(corp_nou);
        S.actualizeazaMatriceFizica();
        E.sincronizeazaMemorie(S);
        // Ramane activ — poti plasa mai multe corpuri consecutiv
    }

    void clickDreapta(sistem& S, editor& E) override {
        anuleaza(E);
    }

    void anuleaza(editor& E) override {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        if (elementeUI.size() < 1) elementeUI.resize(1);

        auto& fantoma = elementeUI[0];
        fantoma.activa = true;
        fantoma.tip = tip_corp;
        fantoma.x = mouse_x;
        fantoma.y = mouse_y;
        fantoma.phi = 0.0f;
        fantoma.dim1 = dimensiune1;
        fantoma.dim2 = dimensiune2;
        fantoma.col = {col.r, col.g, col.b, 0.5f}; // semi-transparent
    }
};
