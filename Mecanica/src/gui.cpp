#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gui.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "grafica.h"
//#include "editor.h"

void setupGUI(GLFWwindow* window){
    IMGUI_CHECKVERSION();
    ImPlot::CreateContext();
    ImGui::CreateContext();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]         = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.00f, 0.60f, 0.00f, 1.00f); // Portocaliu
    style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.FrameRounding = 3.0f;
    style.WindowRounding = 6.0f;
    ImGuiIO& io = ImGui::GetIO();
    
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x2100, 0x218F, // Letterlike Symbols + Number Forms
        0x2190, 0x21FF, // Arrows (Aici sunt sagetile tale)
        0,
    };
    
    ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter_18pt-Medium.ttf", 18.0f, NULL, ranges);
    if (font == NULL) {
        // Dacă nu găsește fișierul, ImGui va folosi fontul default automat.
        std::cerr << "[EROARE] Nu s-a putut incarca fontul din assets/fonts/!" << std::endl;
    }
    
    // Apelat DUPĂ openGLWindow, ImGui va păstra callback-urile tale și le va rula și pe ale sale.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void startFrameGUI(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void renderPanouDeControl(sistem &S, editor &E) {
    ImGuiIO& io = ImGui::GetIO(); // O luam la inceput pentru a o avea disponibila peste tot

    if (E.elementeUI.size() < 2) E.elementeUI.resize(2);
    auto &fantoma_corp = E.elementeUI[0];
    
    ImGui::Begin("Panou de Control Mecanica");
    
    // --- 1. SETARI GENERALE & PLAY/PAUSE ---
    ImGui::Text("Timp simulat: %.3f secunde", E.t);
    
    // Buton dinamic care schimba starea simularii
    if (E.mod_curent == 0) {
        if (ImGui::Button("PAUZA [space] ", ImVec2(-1, 30))) {
            E.mod_curent = 1; 
        }
    } else {
        if (ImGui::Button("PLAY [space]", ImVec2(-1, 30))) {
            E.mod_curent = 0;
        }
    }
    
    ImGui::Separator();
    //ImGui::SliderFloat("Pas de timp (dt)", &dt, 0.0001f, 0.01f, "%.4f");
    
    ImGui::Checkbox("Afiseaza energie", &(E.flag.arata_energie));
    if (E.flag.arata_energie) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Energie: %.3f KJ",S.energie / 1000.0f);
    } 
    
    ImGui::Checkbox("Arata Forte", &(E.flag.arata_forte));
    
    ImGui::Text("Mod De Afisare :");
    ImGui::RadioButton("Standard", &E.mod_vizualizare, 0);  ImGui::SameLine();
    ImGui::RadioButton("Viteze", &E.mod_vizualizare, 1);  ImGui::SameLine();
    ImGui::RadioButton("Acceleratii", &E.mod_vizualizare, 2);
    
    ImGui::Separator();
    
    static float masa = 5.0f;
    static float dimensiuni[2] = { 1.0f, 1.0f };
    static int forma_selectata = 0; 
    static bool meniu_deschis = false;
    static float culoare_aleasa[3] = { 0.3f, 0.6f, 0.9f };
    
    // --- 2. MENIU ADAUGARE (Disponibil doar cand e pe Pauza) ---
    if (E.mod_curent == 1) {
        if (ImGui::Button(meniu_deschis ? "Anuleaza Adaugarea" : "Adauga Corp Nou", ImVec2(-1, 30))) {
            meniu_deschis = !meniu_deschis;
        }
        
        if (meniu_deschis) {
            ImGui::Spacing();
            ImGui::Indent();
            
            ImGui::InputInt("Layer Activ", &E.cadru_activ);
            if (E.cadru_activ < 0) E.cadru_activ = 0;
            
            ImGui::InputFloat("Masa", &masa);
            if (masa <= 0.01f) masa = 0.01f; 
            
            ImGui::Text("Setari Forma:");
            const char* tipuri_forme[] = { "Cerc", "Dreptunghi" };
            if(ImGui::Combo("Tip", &forma_selectata, tipuri_forme, IM_ARRAYSIZE(tipuri_forme))){
                dimensiuni[0] = 1.0f;
                dimensiuni[1] = 1.0f;
                fantoma_corp.phi = 0.0f;
                fantoma_corp.col = {1.0f , 0.75f, 0.80f};
            }
            
            if (forma_selectata == 0) { 
                ImGui::InputFloat("Raza", &dimensiuni[0]); 
            } else { 
                ImGui::InputFloat2("Dimensiuni (L, h)", dimensiuni); 
            }
            
            ImGui::ColorEdit3("Culoare Corp", culoare_aleasa);
            ImGui::Unindent();
            
            ImGui::Spacing();
            ImGui::PushButtonRepeat(true);
            if(ImGui::Button(u8"↺")) fantoma_corp.phi += 0.05f;; 
            ImGui::SameLine() ; 
            if(ImGui::Button(u8"↻")) fantoma_corp.phi += 0.05f;
            ImGui::PopButtonRepeat();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "Click Stanga = Plasare | Click Dreapta = Anulare");
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "Apasa Q / E pentru a roti corpul.");
            
        }
    } else {
        meniu_deschis = false; // Se inchide automat cand dai PLAY
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Opreste simularea pentru a adauga corpuri.");
    }
    
    ImGui::End(); // Panoul s-a inchis aici
    
    // --- 3. LOGICA FANTOMEI ---
    
    if (meniu_deschis && E.mod_curent == 1) {
        // Actualizam datele formei in timp real din slidere
        fantoma_corp.tip = (forma_selectata == 0) ? 1 : 2;
        fantoma_corp.dim1 = dimensiuni[0]; 
        fantoma_corp.dim2 = dimensiuni[1];
        if (forma_selectata == 0) 
        fantoma_corp.dim2 = dimensiuni[0];
        else 
        fantoma_corp.dim2 = dimensiuni[1];
        
        // Verificam daca suntem cu mouse-ul pe scena fizica (nu in meniu)
        if (!io.WantCaptureMouse) {
            fantoma_corp.activa = true;
            fantoma_corp.x = E.mouse_x;
            fantoma_corp.y = E.mouse_y;
            
            if (ImGui::IsKeyDown(ImGuiKey_Q)) fantoma_corp.phi += 0.05f;
            if (ImGui::IsKeyDown(ImGuiKey_E)) fantoma_corp.phi -= 0.05f;
            
            bool se_suprapune = false;
            
            float fRazaX, fRazaY;
            if (fantoma_corp.tip == 1) { // Cerc
                fRazaX = fRazaY = fantoma_corp.dim1;
            } else { // Dreptunghi (calculam raza proiectata pentru rotatie)
                fRazaX = std::abs(std::cos(fantoma_corp.phi) * fantoma_corp.dim1/2.0f) + std::abs(std::sin(fantoma_corp.phi) * fantoma_corp.dim2/2.0f);
                fRazaY = std::abs(std::sin(fantoma_corp.phi) * fantoma_corp.dim1/2.0f) + std::abs(std::cos(fantoma_corp.phi) * fantoma_corp.dim2/2.0f);
            }
            
            for (auto &corp : S.corpuri) {
                if (!corp.activ) continue;
                
                // Verificam DOAR corpurile de pe acelasi layer
                if (corp.collider.cadru == E.cadru_activ) {
                    float dist_x = std::abs(fantoma_corp.x - corp.pozitie.x);
                    float dist_y = std::abs(fantoma_corp.y - corp.pozitie.y);
                    
                    // Verificare AABB (Intersectare Scara Larga)
                    if (dist_x < (fRazaX + corp.collider.bb.razaLatime) && 
                    dist_y < (fRazaY + corp.collider.bb.razaInaltime)) {
                        se_suprapune = true;
                        break;
                    }
                }
            }
            
            // Aplicam culoarea fantomei: Rosie daca se suprapune, altfel culoarea aleasa (cu transparenta)
            if (se_suprapune) {
                fantoma_corp.col = { 1.0f, 0.0f, 0.0f, 0.6f }; // Rosu semitransparent
                ImGui::SetNextWindowPos(ImVec2(io.MousePos.x + 20, io.MousePos.y + 20));
                ImGui::Begin("TooltipSuprapunere", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Nu poti suprapune corpurile!");
                ImGui::End();
            } else {
                fantoma_corp.col = { culoare_aleasa[0], culoare_aleasa[1], culoare_aleasa[2], 0.5f };
            }
            
            // Plasare
            if (ImGui::IsMouseClicked(0) && !se_suprapune)
            {   
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
                S.actualizeazaMatriceFizica();
                E.sincronizeazaMemorie(S);
            }
        } else {
            // Ascundem fantoma complet daca mouse-ul sta pe meniu
            fantoma_corp.activa = false;
        }
        
        // Anulare manuala
        if (ImGui::IsMouseClicked(1) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            meniu_deschis = false;
            fantoma_corp.activa = false;
        }
    } else {
        // Asigurare ca ramane oprita in alte moduri
        fantoma_corp.activa = false; 
    }
}
void renderInspector(sistem &S, editor &E){
    ImGui::Begin("Proprietati");
    
    if(E.corpuriSelectate.empty()){
        ImGui::End();
        return;
    }
    bool trebuie_update = false;
    static bool afiseazaDetalii = 1;  // 0--diagrama 1--grafic
    
    // proprietati pt fiecare corp
    
    for(int i = 0; i < E.corpuriSelectate.size(); i++  ){
        int id_corp = E.corpuriSelectate[i];
        rigid &r = S.corpuri[id_corp];
        
        ImGui::PushID(id_corp);
        // Lumea (corpul 0) are masa infinita, nu vrem sa ii stricam matematica
        if (r.M > 1e10f) {
            ImGui::Text("Tip: Lumea (Element Fix)");
            ImGui::Text("Pozitie: %.2f, %.2f", r.pozitie.x, r.pozitie.y);
        } else {
            bool trebuie_update = false;
            
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),"Corp ID: %d | Tip: %s", id_corp,  (r.collider.tip == 1) ? "Cerc" : "Dreptunghi");
            if (ImGui::DragFloat2("Pozitie (X,Y)", &r.pozitie.x, 0.05f)) trebuie_update = true;
            
            float unghi_grade = r.phi * (180.0f / M_PI);
            if (ImGui::DragFloat("Rotatie (Rad)", &unghi_grade, 0.05f)) {
                trebuie_update = true;
                r.phi = unghi_grade * (M_PI / 180.0f);
            } 
            
            if (ImGui::DragFloat2("Viteza (X,Y)", &r.viteza.x, 0.1f)) trebuie_update = true;
            
            float omega_grade = r.omega * (180.0f / M_PI);
            if (ImGui::DragFloat("Viteza Ungh.", &r.omega, 0.1f)) {
                r.omega = omega_grade * (M_PI / 180.0f); 
                trebuie_update = true;
            }
            
            if (ImGui::DragFloat("Masa", &r.M, 0.5f, 0.1f, 1000.0f)) trebuie_update = true;
            if (ImGui::DragFloat("Inertie", &r.J, 0.5f, 0.1f, 1000.0f)) trebuie_update = true;
        }
        
        ImGui::Separator();
        ImGui::PopID();
    }
    
    if(trebuie_update){
        S.actualizeazaMatriceFizica();
    }

    ImGui::Checkbox("grafic",&afiseazaDetalii);
    
    
    if(!afiseazaDetalii){
    //diagrama 
    ImGui::Text(E.corpuriSelectate.size() == 1 ? "Diagrama de Corp Liber:" : "Diagrama de Grup:");
    
    float min_x = 99999.0f, max_x = -99999.0f, min_y = 99999.0f, max_y = -99999.0f;
    for (int i = 0; i < E.corpuriSelectate.size(); i++) {
        rigid &r = S.corpuri[E.corpuriSelectate[i]];
        if(r.pozitie.x < min_x) min_x = r.pozitie.x;
        if(r.pozitie.x > max_x) max_x = r.pozitie.x;
        if(r.pozitie.y < min_y) min_y = r.pozitie.y;
        if(r.pozitie.y > max_y) max_y = r.pozitie.y;
    }
    vec2 centru_grup((min_x + max_x) / 2.0f, (min_y + max_y) / 2.0f);
    
    float extensie_maxima = 0.5f;
    for( int i = 0; i < E.corpuriSelectate.size(); i++){
        rigid &r  = S.corpuri[E.corpuriSelectate[i]];
        
        vec2 offset_fata_de_centru = r.pozitie - centru_grup;
        float dist_la_centru_corp = offset_fata_de_centru.modul();
        
        float ext_geometrica = (r.collider.tip == 1) ? r.collider.dimensiune1 : std::sqrt(std::pow(r.collider.dimensiune1/2.0f, 2) + std::pow(r.collider.dimensiune2/2.0f, 2));
        
        if (dist_la_centru_corp + ext_geometrica > extensie_maxima) {
            extensie_maxima = dist_la_centru_corp + ext_geometrica;
        }
        
        for(int j = 0; j < r.forte_desen.forte.size(); j++) {
            float mod_f = r.forte_desen.forte[j].valoare.modul();
            if (mod_f < 0.1f) continue;
            
            float lungime_vizuala = std::log10(1.0f + mod_f) * 0.5f;
            vec2 varf_global = r.forte_desen.forte[j].punct_aplicare + (r.forte_desen.forte[j].valoare / mod_f) * lungime_vizuala;
            
            float dist_varf = (varf_global - centru_grup).modul();
            if (dist_varf > extensie_maxima) extensie_maxima = dist_varf;
        }
    }
    
    ImVec2 p0 = ImGui::GetCursorScreenPos(); 
    ImVec2 sz = ImVec2(250.0f, 250.0f);      
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGui::Dummy(sz); 
    
    float scala_grafic = ((sz.x / 2.0f) - 25.0f) / extensie_maxima;
    ImVec2 center = ImVec2(p0.x + sz.x * 0.5f, p0.y + sz.y * 0.5f);
    
    draw_list->AddRectFilled(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(40, 40, 40, 255));
    draw_list->AddRect(p0, ImVec2(p0.x + sz.x, p0.y + sz.y), IM_COL32(100, 100, 100, 255));
    
    for (int id_corp = 0; id_corp < E.corpuriSelectate.size(); id_corp++) {
        rigid &r = S.corpuri[E.corpuriSelectate[id_corp]];
        ImU32 col_corp = IM_COL32((int)(r.collider.culoare.r * 255), (int)(r.collider.culoare.g * 255), (int)(r.collider.culoare.b * 255), 255);
        
        vec2 offset_corp = r.pozitie - centru_grup;
        ImVec2 poz_centru_corp = ImVec2(center.x + offset_corp.x * scala_grafic, center.y - offset_corp.y * scala_grafic);
        
        // Desenare Forma
        if (r.collider.tip == 1) { // CERC
            draw_list->AddCircleFilled(poz_centru_corp, r.collider.dimensiune1 * scala_grafic, col_corp);
        } 
        else if (r.collider.tip == 2) { // DREPTUNGHI
            std::vector<vec2> v_local = {
                vec2(-r.collider.dimensiune1/2.0f, -r.collider.dimensiune2/2.0f),
                vec2( r.collider.dimensiune1/2.0f, -r.collider.dimensiune2/2.0f),
                vec2( r.collider.dimensiune1/2.0f,  r.collider.dimensiune2/2.0f),
                vec2(-r.collider.dimensiune1/2.0f,  r.collider.dimensiune2/2.0f)
            };
            float c = cos(r.phi), s = sin(r.phi);
            for(int i = 0; i < 4; i++) {
                float xr = v_local[i].x * c - v_local[i].y * s;
                float yr = v_local[i].x * s + v_local[i].y * c;
                draw_list->PathLineTo(ImVec2(poz_centru_corp.x + xr * scala_grafic, poz_centru_corp.y - yr * scala_grafic));
            }
            draw_list->PathFillConvex(col_corp); 
        }
        
        // Desenare Forte
        for(int j = 0; j < r.forte_desen.forte.size(); j++) {
            fortaVizuala f = r.forte_desen.forte[j];
            float mod_f = f.valoare.modul();
            if (mod_f < 0.1f) continue;
            
            vec2 offset_sageata = f.punct_aplicare - centru_grup;
            ImVec2 coada = ImVec2(center.x + offset_sageata.x * scala_grafic, center.y - offset_sageata.y * scala_grafic);
            
            vec2 dir_f = f.valoare / mod_f;
            float l_metri = std::log10(1.0f + mod_f) * 0.5f;
            ImVec2 varf = ImVec2(coada.x + dir_f.x * l_metri * scala_grafic, coada.y - dir_f.y * l_metri * scala_grafic);
            
            ImU32 col_forta = IM_COL32(255, 255, 255, 255);
            if(f.tip == FORTA_GREUTATE) col_forta = IM_COL32(50, 200, 50, 255);
            else if(f.tip == FORTA_ELASTICA) col_forta = IM_COL32(50, 120, 255, 255);
            else if(f.tip == FORTA_REACTIUNE) col_forta = IM_COL32(255, 120, 0, 255);
            else if(f.tip == FORTA_IMPACT_NORMAL) col_forta = IM_COL32(0, 255, 255, 255);
            else if(f.tip == FORTA_IMPACT_FRECARE) col_forta = IM_COL32(255, 255, 0, 255);
            
            float grosime = 1.9f + 2.0f * std::log10(1.0f + std::sqrt(std::pow(varf.x-coada.x, 2) + std::pow(varf.y-coada.y, 2)));
            draw_list->AddLine(coada, varf, col_forta, grosime);
            
            float unghi = std::atan2(varf.y - coada.y, varf.x - coada.x);
            float l_varf = 2.0f + 5.0f * std::log10(1.0f + std::sqrt(std::pow(varf.x-coada.x, 2) + std::pow(varf.y-coada.y, 2)));
            draw_list->AddTriangleFilled(varf, 
                ImVec2(varf.x - l_varf * std::cos(unghi + 0.5f), varf.y - l_varf * std::sin(unghi + 0.5f)), 
                ImVec2(varf.x - l_varf * std::cos(unghi - 0.5f), varf.y - l_varf * std::sin(unghi - 0.5f)), 
                col_forta);
            }
        }
    } else if(afiseazaDetalii == 1){

        for(int i = 0; i < E.corpuriSelectate.size(); i++){
            int id_corp = E.corpuriSelectate[i];
            
            if (E.valoriSimulate.size() <= id_corp || E.valoriSimulate[id_corp].timpAfisat.empty()) continue;
            
            IstoricCorp& istoric = E.valoriSimulate[id_corp];
            
            char titlu_grafic[64];
            snprintf(titlu_grafic, sizeof(titlu_grafic), "Parametrii Corp %d###Parametrii%d", id_corp, id_corp);
            
            if (ImPlot::BeginPlot(titlu_grafic)) {
                ImPlotSpec spec;
                spec.Offset = istoric.offset;

                ImPlot::PlotLine("Pozitie X",  istoric.timpAfisat.data(),  istoric.axe[POZITIE_X].data(),  istoric.timpAfisat.size(), spec);   
                ImPlot::PlotLine("Pozitie Y",  istoric.timpAfisat.data(),  istoric.axe[POZITIE_Y].data(),  istoric.timpAfisat.size(), spec);
                ImPlot::PlotLine("Unghi Phi",  istoric.timpAfisat.data(),  istoric.axe[POZITIE_PHI].data(),  istoric.timpAfisat.size(), spec);

                ImPlot::PlotLine("Viteza X",  istoric.timpAfisat.data(),  istoric.axe[VITEZA_X].data(),  istoric.timpAfisat.size(), spec);   
                ImPlot::PlotLine("Viteza Y",  istoric.timpAfisat.data(),  istoric.axe[VITEZA_Y].data(),  istoric.timpAfisat.size(), spec);
                ImPlot::PlotLine("Viteza Unghiulara",  istoric.timpAfisat.data(),  istoric.axe[VITEZA_OMEGA].data(),  istoric.timpAfisat.size(), spec);          
           
                ImPlot::PlotLine("Acceleratie X",  istoric.timpAfisat.data(),  istoric.axe[ACCELERATIE_X].data(),  istoric.timpAfisat.size(), spec);   
                ImPlot::PlotLine("Acceleratie Y",  istoric.timpAfisat.data(),  istoric.axe[ACCELERATIE_Y].data(),  istoric.timpAfisat.size(), spec);
                ImPlot::PlotLine("Acceleratie Unghiulara",  istoric.timpAfisat.data(),  istoric.axe[ACCELERATIE_EPSILON].data(),  istoric.timpAfisat.size(), spec);
                
                ImPlot::EndPlot();
            }
        }
    }
        //butonul de stergere
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); 
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        
        if (ImGui::Button(E.corpuriSelectate.size() == 1 ? "Elimina Corp" : "Elimina Toate Corpurile", ImVec2(-1, 30))) {
            for (int i = 0; i < E.corpuriSelectate.size(); i++) {
                S.eliminaCorp(E.corpuriSelectate[i]);
            }
            E.corpuriSelectate.clear();
            E.sincronizeazaMemorie(S);
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::End();
    }
    
    void endFrameGUI(){
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    
    void cleanupGUI() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

 GLFWwindow*  initializareGrafica(sistem &S, editor &E){
 
    std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
    GLFWwindow* window = openGLWindow(E.shaderProgram);

    if(window != NULL){

        initBuffers(E.VAO, E.VBO);

        setupGUI(window);

        // 17 valori (x, y, phi, w, h, type, red, green, blue, alpha, tip, viteza_x, viteza_x, omega, acceleratie_x, acceleratie_y, epsilon) * (nr_corpuri + nr_legaturi)
        E.vertexBuffer.resize(17 * (S.corpuri.size() + S.legaturi.size() + S.arcuri.size()));

        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        size_t total_elemente =   S.corpuri.size() + S.legaturi.size() + S.arcuri.size();
        E.vertexBuffer.resize(E.vertexStride * total_elemente);
    }
    return window;

}

void randareGrafica(sistem &S, editor &E,  GLFWwindow* &window){
    
    size_t nr_forte = 0;
    if (E.flag.arata_forte) {
        for (const auto& corp : S.corpuri) {
            if (corp.activ && corp.M < 1e10f) {
                nr_forte += corp.forte_desen.forte.size();
            }
        }
    }
    
    size_t total_elemente = S.corpuri.size() + S.legaturi.size() + S.arcuri.size() + E.elementeUI.size() + nr_forte;

    (E.vertexBuffer).resize(17 * total_elemente);
    drawSystem(S,E, E.VAO, E.VBO, E.shaderProgram, E.vertexBuffer.data());

    renderPanouDeControl(S,E);
    if(E.corpuriSelectate.size() != 0)
       renderInspector(S,E);
    endFrameGUI();

    glfwSwapBuffers(window);
    glfwPollEvents();

    if(E.mod_curent == MOD_RULARE)
        (E.frameCount)++;
    if(E.frameCount % 60 == 0) { 
        //std::cout << "\n[Fizica] Cadru " << frameCount << " | Timp: " << t << " | Corpuri in sistem: " << S.corpuri.size() << std::endl;
        for (size_t c = 0; c < S.corpuri.size(); c++) {
            if(S.corpuri[c].activ == 0) continue; // Sarim peste cele sterse
            //std::cout << "  -> Corp " << c 
            //          << " | Pos: (" << S.corpuri[c].pozitie.x << ", " << S.corpuri[c].pozitie.y << ")"
            //          << " | Viteza: (" << S.corpuri[c].viteza.x << ", " << S.corpuri[c].viteza.y << ")"
            //          << " | Masa: " << S.corpuri[c].M << std::endl;
            //std::cout <<" corpul : "<< c;
            // for(int j  =  0; j < S.corpuri[c].forte.size(); j++){
            //    
            //    fortaExterna f = S.corpuri[c].forte[j];
            //    std::cout<< f.u.x * f.modul << " | " << f.u.y * f.modul;
            //}
            //std::cout << std::endl;
            }
    }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
}