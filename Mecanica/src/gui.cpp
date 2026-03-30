#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "grafica.h"
//#include "editor.h"

void setupGUI(GLFWwindow* window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    
    // Apelat DUPĂ openGLWindow, ImGui va păstra callback-urile tale și le va rula și pe ale sale.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void startFrameGUI(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void renderPanouDeControl(float &dt, bool &running_flag, bool &arata_energie_flag,  bool &arata_forte_flag, float t, float energie) {
    ImGui::Begin("Panou de Control Mecanica");
    ImGui::Text("Timp simulat: %.3f secunde", t);
    ImGui::Separator();
    ImGui::Checkbox("Ruleaza simularea", &running_flag);
    ImGui::SliderFloat("Pas de timp (dt)", &dt, 0.0001f, 0.01f, "%.4f");
    ImGui::Checkbox("Afiseaza energie", &arata_energie_flag);
    ImGui::Checkbox("Arata Forte", &arata_forte_flag);
    if (arata_energie_flag) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Energie: %.3f KJ", energie / 1000.0f);
    }
    ImGui::End();
}

void renderPanouDeAdaugatCorpuri(sistem &S, editor &E) {
    if (E.elementeUI.size() < 2) {
        E.elementeUI.resize(2);
    }
    
    auto &fantoma_corp = E.elementeUI[0];
    auto &fantoma_legatura = E.elementeUI[1];

    static float masa = 5.0f;
    static float dimensiuni[2] = { 1.0f, 1.0f };
    static int forma_selectata = 0; 
    ImGuiIO& io = ImGui::GetIO();

    if (fantoma_corp.activa) {
        fantoma_corp.x = E.mouse_x;
        fantoma_corp.y = E.mouse_y;

        if (ImGui::IsKeyDown(ImGuiKey_Q)) fantoma_corp.phi += 0.05f;
        if (ImGui::IsKeyDown(ImGuiKey_E)) fantoma_corp.phi -= 0.05f;

        if (ImGui::IsMouseClicked(0) && !io.WantCaptureMouse) {
            rigid forma_noua;
            if (fantoma_corp.tip == 1) {
                forma_noua = rigid::Disc(fantoma_corp.x, fantoma_corp.y, fantoma_corp.dim1, masa);
            } else {
                forma_noua = rigid::Bara(fantoma_corp.x, fantoma_corp.y, fantoma_corp.dim1, fantoma_corp.dim2, masa);
            }
            forma_noua.phi = fantoma_corp.phi;
            forma_noua.collider.culoare = {fantoma_corp.col.r, fantoma_corp.col.g, fantoma_corp.col.b, 1.0f};
            forma_noua.collider.cadru = E.cadru_activ;
            
            S.adaugaCorpuri(forma_noua);
            S.incarcaStare();           
            S.seteazaMatriceInertie();  
            S.seteazaJacobian();
            S.seteazaConstrangeri();
            S.seteazaForteExterne();
            fantoma_corp.activa = false;
        }
        else if (ImGui::IsMouseClicked(1) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            fantoma_corp.activa = false;
        }
    }

    ImGui::Begin("Panou de Adaugat Corpuri");
    ImGui::Text("Mod de utilizare:");
    ImGui::RadioButton("Interactiune", &E.mod_curent, 0); ImGui::SameLine();
    ImGui::RadioButton("Editare", &E.mod_curent, 1);

    if (E.mod_curent == 1) {
        ImGui::Separator();
        ImGui::InputInt("Layer Activ", &E.cadru_activ);
        if (E.cadru_activ < 0) E.cadru_activ = 0;

        ImGui::InputFloat("Masa", &masa);
        if (masa <= 0.01f) masa = 0.01f; 

        bool panou_blocat = fantoma_corp.activa;
        if (panou_blocat) ImGui::BeginDisabled();

        ImGui::Separator();
        ImGui::Text("Adauga Forma Noua:");
        const char* tipuri_forme[] = { "Cerc", "Dreptunghi" };
        ImGui::Combo("Tip", &forma_selectata, tipuri_forme, IM_ARRAYSIZE(tipuri_forme));

        if (forma_selectata == 0) { ImGui::InputFloat("Raza", &dimensiuni[0]); } 
        else { ImGui::InputFloat2("Dimensiuni (L, h)", dimensiuni); }

        if (ImGui::Button("Pregateste Forma")) {
            fantoma_corp.activa = true;
            fantoma_corp.tip = (forma_selectata == 0) ? 1 : 2;
            fantoma_corp.dim1 = dimensiuni[0]; fantoma_corp.dim2 = dimensiuni[1];
            if (forma_selectata == 0) fantoma_corp.col = {0.3f, 0.9f, 0.3f, 0.5f};
            else fantoma_corp.col = {0.9f, 0.3f, 0.9f, 0.5f};
        }
        
        if (panou_blocat) ImGui::EndDisabled();
    }
    ImGui::End();
}

void renderInspector(sistem &S, editor &E) {
    ImGui::Begin("Proprietati");

    if (E.corpApasat != -1 && E.corpApasat < S.corpuri.size()) {
        rigid &r = S.corpuri[E.corpApasat];
        
        // Lumea (corpul 0) are masa infinita, nu vrem sa ii stricam matematica
        if (r.M > 1e10f) {
            ImGui::Text("Tip: Lumea (Element Fix)");
            ImGui::Text("Pozitie: %.2f, %.2f", r.pozitie.x, r.pozitie.y);
        } else {
            bool trebuie_update = false;

            // Daca utilizatorul trage de slider, functia returneaza true
            if (ImGui::DragFloat2("Pozitie (X, Y)", &r.pozitie.x, 0.05f)) trebuie_update = true;
            if (ImGui::DragFloat("Rotatie (Rad)", &r.phi, 0.05f)) trebuie_update = true;
            
            ImGui::Spacing();
            
            if (ImGui::DragFloat2("Viteza (X, Y)", &r.viteza.x, 0.1f)) trebuie_update = true;
            if (ImGui::DragFloat("Viteza Ungh.", &r.omega, 0.1f)) trebuie_update = true;
            
            ImGui::Separator();

            if (ImGui::DragFloat2("Forta (X, Y)", &r.tau.forta.x ,0.1f)) trebuie_update = true;
            if (ImGui::DragFloat("Moment", &r.tau.moment , 0.1f)) trebuie_update = true;
            
            ImGui::Separator();
            
            // Masa si Inerție - nu le lasam sa scada sub 0.1 ca sa nu dea erori de impartire la 0
            if (ImGui::DragFloat("Masa", &r.M, 0.5f, 0.1f, 1000.0f)) trebuie_update = true;
            if (ImGui::DragFloat("Inertie", &r.J, 0.5f, 0.1f, 1000.0f)) trebuie_update = true;

            // Sincronizarea cu motorul RK4
            if (trebuie_update) {
                S.incarcaStare();           // Copiaza noile pozitii/viteze inapoi in vectorul "stare"
                S.seteazaMatriceInertie();  // Recalculeaza A si A^-1 in caz ca am modificat masa
            }
        }

        ImGui::Separator();
            ImGui::Text("Diagrama de Corp Liber:");

            // 1. Definim "panza" de desenare din interiorul ferestrei
            ImVec2 p0 = ImGui::GetCursorScreenPos(); // Coltul stanga-sus al zonei de desen
            ImVec2 sz = ImVec2(250.0f, 250.0f);      // Cat de mare sa fie casuta
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
            // Factori de scalare ca sa incapa frumos in casuta

            // 1. Aflăm extensia geometrică a corpului (în metri)
                float extensie_maxima = 0.1f; 

                if (r.collider.tip == CERC) {
                extensie_maxima = r.collider.dimensiune1; 
                } 
                else if (r.collider.tip == DREPTUNGHI) {
                    float w = r.collider.dimensiune1 / 2.0f;
                    float h = r.collider.dimensiune2 / 2.0f;
                extensie_maxima = std::sqrt(w * w + h * h);
                }

                // 2. Extindem "cutia" de încadrare dacă forțele ies în afară
                // Setăm câți Newtoni reprezintă vizual lungimea de 1 metru pe ecran
                    float raport_forta_la_metru = 0.05f ; // 100N = 1 metru lungime vizuală

                for(int j = 0; j < r.forte_desen.forte.size(); j++) {
                    fortaVizuala f = r.forte_desen.forte[j];
                    float mod_f = f.valoare.modul();
                    if (mod_f < 0.1f) continue;
    
                    // Distanța de la centru la punctul de aplicare
                    vec2 offset_global = f.punct_aplicare - r.pozitie;
                    vec2 dir_f = f.valoare / mod_f;
                    float lungime_vizuala_metri = std::log10(1.0f + mod_f) * 0.5f; 
    
                    vec2 varf_local = offset_global + dir_f * lungime_vizuala_metri;
    
                 float distanta_varf = varf_local.modul();
                if (distanta_varf > extensie_maxima) {
                    extensie_maxima = distanta_varf;
                }
            }

            // 3. Calculăm multiplicatorii finali astfel încât cel mai depărtat punct să atingă marginea
            float raza_maxima_pixeli = (sz.x / 2.0f) - 25.0f; 
              
            float scala_grafic = raza_maxima_pixeli / extensie_maxima;
            float scala_forte = scala_grafic * raport_forta_la_metru;
            ImU32 culoare_ui = IM_COL32(40, 40, 40, 255);

            // Rezervam spatiul ca ImGui sa nu scrie text peste el
            ImGui::Dummy(sz); 

            // Fundal gri inchis pentru diagrama
            draw_list->AddRectFilled(p0, ImVec2(p0.x + sz.x, p0.y + sz.y),culoare_ui );
            draw_list->AddRect(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(100, 100, 100, 255)); // Contur

            // Centrul panzei
            ImVec2 center = ImVec2(p0.x + sz.x * 0.5f, p0.y + sz.y * 0.5f);



            // 2. Desenam Corpul
            ImU32 col_corp = IM_COL32((int)(r.collider.culoare.r * 255), (int)(r.collider.culoare.g * 255), (int)(r.collider.culoare.b * 255), 255);

            if (r.collider.tip == CERC) {
                draw_list->AddCircleFilled(center, r.collider.dimensiune1 * scala_grafic, col_corp);
            } 
            else if (r.collider.tip == DREPTUNGHI) { // Pe viitor aici va fi: || r.collider.tip == POLIGON_CONVEX
                
                // Aici pregatim varfurile locale (in jurul centrului 0,0 al corpului)
                std::vector<vec2> varfuri_locale;
                
                if (r.collider.tip == DREPTUNGHI) {
                    float w = r.collider.dimensiune1 / 2.0f;
                    float h = r.collider.dimensiune2 / 2.0f;
                    varfuri_locale.push_back(vec2(-w, -h));
                    varfuri_locale.push_back(vec2( w, -h));
                    varfuri_locale.push_back(vec2( w,  h));
                    varfuri_locale.push_back(vec2(-w,  h));
                }
                // Cand adaugi poligoane, vei adauga doar atat:
                /* else if (r.collider.tip == POLIGON_CONVEX) {
                    varfuri_locale = r.vectorul_tau_de_varfuri_din_clasa;
                } */

                // Desenam poligonul (Oricat de multe varfuri ar avea!)
                float c = cos(r.phi); 
                float s = sin(r.phi);
                
                for(int i = 0; i < varfuri_locale.size(); i++) {
                    // Rotim varful
                    float x_rotit = varfuri_locale[i].x * c - varfuri_locale[i].y * s;
                    float y_rotit = varfuri_locale[i].x * s + varfuri_locale[i].y * c;
                    
                    // Il skalam si il punem in centrul radarului
                    ImVec2 punct_ecran = ImVec2(center.x + x_rotit * scala_grafic, center.y - y_rotit * scala_grafic);
                    draw_list->PathLineTo(punct_ecran); // Spunem ImGui-ului pe unde sa treaca linia
                }
                
                // ImGui uneste automat ultimul punct cu primul si coloreaza interiorul
                draw_list->PathFillConvex(col_corp); 
            }

            // 3. Desenam Fortele

            auto trageSageataUI = [&](fortaVizuala f, ImU32 culoare) {
                float mod_f = f.valoare.modul();
                if (mod_f < 0.1f) return;

                vec2 offfset_global = f.punct_aplicare - r.pozitie;

                ImVec2 coada = ImVec2(center.x + offfset_global.x *scala_grafic, center.y - offfset_global.y *scala_grafic);
                float lungime_vizuala_metri = std::log10(1.0f + mod_f) * 0.5f;
                vec2 dir_f = f.valoare / mod_f;

                ImVec2 varf = ImVec2(coada.x + dir_f.x * lungime_vizuala_metri * scala_grafic, coada.y - dir_f.y * lungime_vizuala_metri * scala_grafic);
                float dx = varf.x - coada.x;
                float dy = varf.y - coada.y;
                
                float lungime = std::sqrt(dx*dx + dy*dy);

                if(lungime < 2.0f) return;

                float factor_log = std::log10(1.0f + lungime);
                float grosime = 1.9f + 2.0f * factor_log;
                draw_list->AddLine(coada, varf, culoare, grosime);

                float unghi = std::atan2(varf.y - coada.y, varf.x - coada.x);
                float lungime_varf = 2.0f + 5.0f*factor_log;
                float deschidere = 0.5f;

                ImVec2 p1 = ImVec2(varf.x - lungime_varf * std::cos(unghi + deschidere), varf.y - lungime_varf * std::sin(unghi + deschidere));
                ImVec2 p2 = ImVec2(varf.x - lungime_varf * std::cos(unghi - deschidere), varf.y - lungime_varf * std::sin(unghi - deschidere));   
            
                draw_list->AddTriangleFilled(varf,p1,p2,culoare);
            };

            for(int j = 0; j < r.forte_desen.forte.size(); j++){

                fortaVizuala f = r.forte_desen.forte[j];
                ImU32 culoare_ui = IM_COL32(255, 255, 255, 255);

                switch(f.tip){
                    case FORTA_GREUTATE:       culoare_ui = IM_COL32(50, 200, 50, 255);   break; // Verde
                    case FORTA_ELASTICA:       culoare_ui = IM_COL32(50, 120, 255, 255);  break; // Albastru
                    case FORTA_REACTIUNE:      culoare_ui = IM_COL32(255, 120, 0, 255);   break; // Portocaliu
                    case FORTA_IMPACT_NORMAL:  culoare_ui = IM_COL32(0, 255, 255, 255);   break; // Cyan
                    case FORTA_IMPACT_FRECARE: culoare_ui = IM_COL32(255, 255, 0, 255);   break; // Galben
                }
                trageSageataUI(f, culoare_ui);
            }
    } 

    ImGui::End();
}

void endFrameGUI(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void cleanupGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}