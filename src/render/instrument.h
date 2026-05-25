#pragma once
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include "sistem.h"
#include "editor.h"

class editor;
struct fantomaUI;

class InstrumentEditor{
public:
    virtual ~InstrumentEditor() = default;

    virtual void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) = 0;
    virtual void clickDreapta(sistem& S, editor& E) = 0;

    virtual void anuleaza(editor& E, sistem &S) = 0;

    virtual void miscareMouse(sistem& S, editor& E, float mouse_x, float mouse_y) {}
    virtual void eliberareClickStanga(sistem& S, editor& E) {}

    virtual void pregatesteFantome(std::vector<fantomaUI> &elementeUI, float mouse_x, float mouse_y, sistem& S) {}
    virtual void deseneazaSetariUI(editor& E,sistem &S) {}

    virtual void randeazaPanouAditional(editor& E, sistem &S) {}

};

class InstrumentSelectie : public InstrumentEditor {
public:

    std::vector<std::pair<int, vec2>> offseturi_drag;

    bool mouse_apasat = false;
    vec2 pozitie_click_initial;
    int id_corp_la_click = -1;
    bool in_drag = false;
    static constexpr float PRAG_DRAG = 0.2f;

    ObiectSelectat element_la_click = {TIP_CORP, -1};

    bool in_box_select = false;
    vec2 box_start;
    vec2 box_curent;

    void actualizeazaSelectatVizual(sistem& S, editor& E) {
       for (auto& corp : S.corpuri) corp.collider.selectat = 0;
        for (auto* leg : S.legaturi) leg->selectat = 0;

        for (const auto& el : E.elementeSelectate) {
            if (el.tip == TIP_CORP && el.id >= 0 && el.id < S.corpuri.size()) {
                S.corpuri[el.id].collider.selectat = 1;
            }
            else if (el.tip == TIP_LEGATURA && el.id >= 0 && el.id < S.legaturi.size()) {
                S.legaturi[el.id]->selectat = 1;
            }
        }
    }

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (ImGui::GetIO().WantCaptureMouse) return;

        mouse_apasat = true;
        in_drag = false;
        in_box_select = false;
        offseturi_drag.clear();
        pozitie_click_initial = vec2(mouse_x, mouse_y);

        element_la_click = E.elementeSubMouse.empty() ? ObiectSelectat{TIP_CORP, -1} : E.elementeSubMouse[0];
        bool shift_apasat = glfwGetKey(E.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        auto& sel = E.elementeSelectate;

        if (element_la_click.id == -1) {
            in_box_select = true;
            box_start = vec2(mouse_x, mouse_y);
            box_curent = box_start;
            
            if (!shift_apasat) {
                sel.clear();
                actualizeazaSelectatVizual(S, E);
            }
            return; 
        }

        if (element_la_click.id != -1 && !(element_la_click.tip == TIP_CORP && element_la_click.id == 0)) {
            auto it = std::find(sel.begin(), sel.end(), element_la_click);
            
            if (!shift_apasat) {
               if (it == sel.end()) {
                    sel.clear();
                    sel.push_back(element_la_click);
                }
            } else {
                if (it == sel.end()) sel.push_back(element_la_click);
                else sel.erase(it);
            }
        } else {
            if (!shift_apasat) {
                sel.clear();
            }
        }

        actualizeazaSelectatVizual(S, E);

        for (const auto& el : E.elementeSelectate) {
            if (el.tip == TIP_CORP && el.id > 0 && el.id < S.corpuri.size()) {
                vec2 offset(S.corpuri[el.id].pozitie.x - mouse_x, S.corpuri[el.id].pozitie.y - mouse_y);
                offseturi_drag.push_back({el.id, offset});
            }
        }

    }

    void miscareMouse(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (!mouse_apasat) return;

        if (in_box_select) {
            box_curent = vec2(mouse_x, mouse_y);
            return; 
        }

        vec2 delta = vec2(mouse_x, mouse_y) - pozitie_click_initial;
        
        float prag_real = std::max(0.05f,PRAG_DRAG * (E.camera.zoom / 10.0f));

        if (!in_drag && delta.modul() > prag_real) {
            in_drag = true;

            if (element_la_click.id == -1 || element_la_click.tip != TIP_CORP || element_la_click.id == 0) {
                offseturi_drag.clear();
            }
        }
        
        if (in_drag && !offseturi_drag.empty()) {
            for (auto& [id, offset] : offseturi_drag) {
                S.corpuri[id].pozitie.x = mouse_x + offset.x;
                S.corpuri[id].pozitie.y = mouse_y + offset.y;
                S.corpuri[id].viteza = vec2(0.0f, 0.0f);
            }
            S.actualizeazaMatriceFizica();
        }
    }

   void eliberareClickStanga(sistem& S, editor& E) override {

        if (in_box_select) {
            float min_x = std::min(box_start.x, box_curent.x);
            float max_x = std::max(box_start.x, box_curent.x);
            float min_y = std::min(box_start.y, box_curent.y);
            float max_y = std::max(box_start.y, box_curent.y);

            for (int i = 1; i < S.corpuri.size(); i++) {
                if (!S.corpuri[i].activ || S.corpuri[i].collider.obiectVirtual) continue;
                
                vec2 p = S.corpuri[i].pozitie;
                if (p.x >= min_x && p.x <= max_x && p.y >= min_y && p.y <= max_y) {
                    ObiectSelectat obj = {TIP_CORP, i};
                    if (std::find(E.elementeSelectate.begin(), E.elementeSelectate.end(), obj) == E.elementeSelectate.end()) {
                        E.elementeSelectate.push_back(obj);
                    }
                }
            }
            actualizeazaSelectatVizual(S, E);
        }

        if (!in_drag && element_la_click.id != -1 && !(element_la_click.tip == TIP_CORP && element_la_click.id == 0)) {
            bool shift_apasat = glfwGetKey(E.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
            if (!shift_apasat) {
                E.elementeSelectate.clear();
                E.elementeSelectate.push_back(element_la_click);
                actualizeazaSelectatVizual(S, E);
            }
        }
        
        mouse_apasat = false;
        in_drag = false;
        offseturi_drag.clear();
        element_la_click = {TIP_CORP, -1};
    }

    void clickDreapta(sistem& S, editor& E) override {
        anuleaza(E, S);
    }

    void anuleaza(editor& E, sistem &S) override {
        E.elementeSelectate.clear();
        actualizeazaSelectatVizual(S, E);
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        
        if (in_box_select) {
            if (elementeUI.size() < 1) elementeUI.resize(1);
            
            elementeUI[0].activa = true;
            elementeUI[0].tip = 2; // Tip 2 este dreptunghi in shaderul tau
            elementeUI[0].x = (box_start.x + box_curent.x) / 2.0f;
            elementeUI[0].y = (box_start.y + box_curent.y) / 2.0f;
            elementeUI[0].dim1 = std::abs(box_curent.x - box_start.x);
            elementeUI[0].dim2 = std::abs(box_curent.y - box_start.y);
            elementeUI[0].phi = 0.0f;
            
            // Un albastru sters, semi-transparent
            elementeUI[0].col = {0.2f, 0.5f, 0.9f, 0.3f}; 
        } else {
            for(auto& f : elementeUI) {
                f.activa = false;
            }
        }
    }

    void randeazaPanouAditional(editor& E, sistem &S) override {
        ;
    }

    void deseneazaSetariUI(editor& E, sistem &S) override {
    if (E.elementeSelectate.empty()) return;

    int n = E.elementeSelectate.size();
 
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
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            if (obj.id != -1 && obj.tip == TIP_CORP) {
                id_corp_A = obj.id;
                punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
                unghi_fantoma = S.corpuri[id_corp_A].phi;
                pas = 1;
            }
        } else if (pas == 1){
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            int id_corp_B = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0;

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
            anuleaza(E,S);
        }
    }

    void anuleaza(editor& E, sistem &S) override {
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

    void deseneazaSetariUI(editor& E,sistem &S) override {
        ;
    }

    
    void randeazaPanouAditional(editor& E, sistem &S) override {
        ;
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
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            if (obj.id != -1 && obj.tip == TIP_CORP) {
                id_corp_A = obj.id;
                punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
                unghi_fantoma = S.corpuri[id_corp_A].phi;
                pas = 1;
            }
        } 
        else if (pas == 1) {
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            int id_corp_B = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0; 

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
            anuleaza(E,S);
        }
    }

    void anuleaza(editor& E, sistem&S) override {
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

    void deseneazaSetariUI(editor& E,sistem &S) override {
        ;
    }
    
    void randeazaPanouAditional(editor& E, sistem &S) override {
        ;
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

    ImVec2 mouse_la_click;

    generatorForte* arc_adaugat = nullptr;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            id_corp_A = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0;

            punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
            pas = 1;
        } else if (pas == 1) {
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            int id_corp_B = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0;

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

            mouse_la_click = ImGui::GetIO().MousePos;

            pas = 2;
            
        } else if (pas == 2){
            ;
        } else if (pas ==3) {
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
            
        } else if (pas == 3) {
            if (arc_adaugat) arc_adaugat->activ = 0;
            E.schimbaInstrumentCurent(new InstrumentSelectie());
            
        } else {
            anuleaza(E,S);
        }
    }

    void anuleaza(editor& E, sistem &S) override {
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

    void deseneazaSetariUI(editor& E,sistem &S) override {
        ImGui::Text("Setari Arc");
        ImGui::Separator();
        
        ImGui::SliderFloat("Rigiditate (k)", &arc_k, 1.0f, 500.0f);
        ImGui::SliderFloat("Amortizare (d)", &arc_d, 0.0f, 20.0f);
        ImGui::SliderFloat("Lungime repaus (%)", &arc_l0_procent, 10.0f, 200.0f);
    }

    
    void randeazaPanouAditional(editor& E, sistem &S) override {
        if (pas == 2) {
            ImGui::SetNextWindowPos(ImVec2(mouse_la_click.x + 15, mouse_la_click.y + 15), ImGuiCond_Always);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |ImGuiWindowFlags_NoSavedSettings;
        
            if (ImGui::Begin("  ", nullptr, flags)) {
                deseneazaSetariUI(E, S);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("Finalizeaza", ImVec2(-1, 0))) {
                    pas = 3; 
                }
            }
            ImGui::End();
            return;
        }
        if (pas == 3) {
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }
};

class InstrumentAdaugaFir : public InstrumentEditor {
public:
    int pas = 0;
    int id_corp_A = -1;
    vec2 punct_A_local;

    float lungime_procent = 100.0f;

    ImVec2 mouse_la_click;

    legatura* fir_adaugat = nullptr;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            id_corp_A = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0;

            punct_A_local = S.corpuri[id_corp_A].globalToLocal(vec2(mouse_x, mouse_y));
            pas = 1;
        } else if (pas == 1) {
            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            int id_corp_B = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0;

            vec2 pA_global = S.corpuri[id_corp_A].localToGlobal(punct_A_local);
            vec2 pB_global(mouse_x, mouse_y);
            
            fir* fir_nou = fir::Creaza(S.corpuri[id_corp_A], S.corpuri[id_corp_B],
                                      pA_global.x, pA_global.y, pB_global.x, pB_global.y);
            
            fir_nou->lungime *= (lungime_procent / 100.0f);

            S.adaugaLegaturi(fir_nou);
            S.actualizeazaMatriceFizica();
            E.sincronizeazaMemorie(S);
            
            fir_adaugat = fir_nou;

            mouse_la_click = ImGui::GetIO().MousePos;

            pas = 2;
        } else if (pas == 2){
            ;
        } else if (pas ==3) {
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
            
        } else if (pas == 3) {
            if (fir_adaugat) fir_adaugat->activ = 0;
            E.schimbaInstrumentCurent(new InstrumentSelectie());
            
        } else {
            anuleaza(E,S);
        }

    }

    void anuleaza(editor& E, sistem &S) override {
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

    void deseneazaSetariUI(editor& E,sistem &S) override {
        ImGui::Text("Setari Fir");
        ImGui::Separator();
        ImGui::SliderFloat("Lungime (%)", &lungime_procent, 10.0f, 200.0f, "%.0f%%");
    }

    
    void randeazaPanouAditional(editor& E, sistem &S) override {
        if (pas == 2) {
            ImGui::SetNextWindowPos(ImVec2(mouse_la_click.x + 15, mouse_la_click.y + 15), ImGuiCond_Always);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |ImGuiWindowFlags_NoSavedSettings;
        
            if (ImGui::Begin("  ", nullptr, flags)) {
                deseneazaSetariUI(E, S);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("Finalizeaza", ImVec2(-1, 0))) {
                    pas = 3; 
                }
            }

            ImGui::End();
            return;
        }

        if (pas == 3) {
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }

};

class InstrumentAdaugaCorp : public InstrumentEditor {
public:
    int tip_corp = DREPTUNGHI;      // CERC sau DREPTUNGHI
    float dimensiune1 = 2.0f;       // Lungime (bara) sau Raza (disc)
    float dimensiune2 = 2.0f;       // Grosime (bara) — ignorat pt disc
    float masa = 1.0f;
    culoare col = {0.4f, 0.8f, 0.4f, 1.0f};
    const char* material = "Lemn";
    float unghi_initial = 0.0f;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        rigid corp_nou;
        if (tip_corp == CERC) {
            corp_nou = rigid::Disc(mouse_x, mouse_y, dimensiune1, masa, material);
        } else {
            corp_nou = rigid::Bara(mouse_x, mouse_y, dimensiune1, dimensiune2, masa, material);
        }
        corp_nou.collider.culoare = col;
        corp_nou.collider.cadru = E.cadru_activ;
        corp_nou.phi = unghi_initial;
        S.adaugaCorpuri(corp_nou);
        S.actualizeazaMatriceFizica();
        E.sincronizeazaMemorie(S);
        // Ramane activ — poti plasa mai multe corpuri consecutiv
    }

    void clickDreapta(sistem& S, editor& E) override {
        anuleaza(E,S);
    }

    void anuleaza(editor& E, sistem &S) override {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        if (elementeUI.size() < 1) elementeUI.resize(1);

        auto& fantoma = elementeUI[0];
        fantoma.activa = true;
        fantoma.tip = tip_corp;
        fantoma.x = mouse_x;
        fantoma.y = mouse_y;
        fantoma.phi = unghi_initial;
        fantoma.dim1 = dimensiune1;
        fantoma.dim2 = dimensiune2;
        fantoma.col = {col.r, col.g, col.b, 0.5f}; // semi-transparent
    }

    void deseneazaSetariUI(editor& E,sistem &S) override {
        ImGui::Text("Setari Corp Nou");
        ImGui::Separator();

        // Dropdown pentru forma
        int tip_sel = (tip_corp == 1) ? 0 : 1;
        const char* tipuri[] = { "Disc", "Dreptunghi" };
        if (ImGui::Combo("Forma", &tip_sel, tipuri, 2)) {
            tip_corp = (tip_sel == 0) ? 1 : 2; 
        }

        if (tip_sel == 0) {
            ImGui::SliderFloat("Raza", &dimensiune1, 0.1f, 10.0f);
        } else {
            ImGui::SliderFloat("Lungime", &dimensiune1, 0.1f, 20.0f);
            ImGui::SliderFloat("Grosime", &dimensiune2, 0.1f, 10.0f);
        }

        ImGui::SliderFloat("Masa", &masa, 0.1f, 100.0f, "%.1f kg");

        float color[4] = {col.r, col.g, col.b, col.a};
        if (ImGui::ColorEdit4("Culoare", color)) {
            col = {color[0], color[1], color[2], color[3]};
        }

        ImGui::Button("↺");
            if( ImGui::IsItemActive() || ImGui::IsKeyDown(ImGuiKey_Q)){
               unghi_initial += 0.05;
            }

        ImGui::SameLine();

        ImGui::Button("↻");
            if( ImGui::IsItemActive() || ImGui::IsKeyDown(ImGuiKey_E) ){
                unghi_initial -= 0.05;
            }
    }

    void randeazaPanouAditional(editor& E, sistem &S) override {
        ;
    }

};


class InstrumentAdaugaMotor : public InstrumentEditor {
public:
    int pas = 0;
    int id_corp = -1;
    vec2 punct_local;
    vec2 punct_global;

    ImVec2 mouse_la_click;

    generatorForte* motor_adaugat = nullptr;

    float moment_tinta = 50.0f;

    void clickStanga(sistem& S, editor& E, float mouse_x, float mouse_y) override {
        if (pas == 0) {

            ObiectSelectat obj = E.gasesteObiectSubMouse(S);
            id_corp = (obj.id != -1 && obj.tip == TIP_CORP) ? obj.id : 0;

            punct_global =  vec2(mouse_x,mouse_y);
            punct_local = S.corpuri[id_corp].globalToLocal(punct_global); 
            
            motor* motor_nou = motor::Creaza(S.corpuri[id_corp], punct_global.x,  punct_global.y, moment_tinta);
            S.adaugaGeneratorForte(motor_nou);
            S.actualizeazaMatriceFizica();
            
            motor_adaugat = motor_nou;

            mouse_la_click = ImGui::GetIO().MousePos;
            pas = 1;
            
        } else if (pas == 1){
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }

    void clickDreapta(sistem& S, editor& E) override {
        anuleaza(E,S);
    }

    void anuleaza(editor& E, sistem &S) override {
        E.elementeUI.clear();
    }

    void pregatesteFantome(std::vector<fantomaUI>& elementeUI, float mouse_x, float mouse_y, sistem& S) override {
        if (elementeUI.size() < 2) elementeUI.resize(2);
        elementeUI[0].activa = false;
        elementeUI[1].activa = false;

        if (pas == 1) {
            
            elementeUI[1].phi = 0.0f;
            elementeUI[1].dim1 = 1.0f ;
            elementeUI[1].dim2 = 1.0f  ;
            elementeUI[1].x = punct_global.x; 
            elementeUI[1].y = punct_global.y;
            elementeUI[1].tip = 1; 
            elementeUI[1].col = {0.5f, 1.0f, 0.5f, 0.8f};
        }
    }

    void deseneazaSetariUI(editor& E, sistem &S) override {
        ImGui::Text("Setari Instrument Motor");
        ImGui::Separator();
        ImGui::Spacing();
        
        // Un slider care permite și valori negative pentru a schimba sensul de rotație implicit
        ImGui::SliderFloat("Cuplu Motor (Nm)", &moment_tinta, -500.0f, 500.0f, "%.1f Nm");
        
        ImGui::Spacing();
        ImGui::TextDisabled("Click stanga pe un corp pentru a-i\ninstala acest motor.");
    }

    
    void randeazaPanouAditional(editor& E, sistem &S) override {
        if (pas == 1) {
            ImGui::SetNextWindowPos(ImVec2(mouse_la_click.x + 15, mouse_la_click.y + 15), ImGuiCond_Always);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |ImGuiWindowFlags_NoSavedSettings;
        
            if (ImGui::Begin("  ", nullptr, flags)) {
                deseneazaSetariUI(E, S);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("Finalizeaza", ImVec2(-1, 0))) {
                    pas = 2; 
                }
            }
            ImGui::End();
            return;
        }
        if (pas == 2) {
            E.schimbaInstrumentCurent(new InstrumentSelectie());
        }
    }
};





