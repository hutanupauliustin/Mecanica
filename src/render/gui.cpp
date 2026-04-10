#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gui.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include "grafica.h"
#include "font.h"
#include <cstring>
#include "portable-file-dialogs.h"
#include "scena.h"

std::string cereLocatieSalvare() {
    auto f = pfd::save_file("Salveaza Export CSV", "export_simulare.csv",
                            { "Fisiere CSV", "*.csv", "Toate Fisierele", "*" });
    
    std::string result = f.result();
    if (!result.empty() && (result.length() < 4 || result.substr(result.length() - 4) != ".csv")) {
        result += ".csv";
    }
    return result;
}

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
    
    ImFont* font = nullptr;
    
    // Verificăm dacă fișierul există înainte să lăsăm ImGui să încerce să-l încarce
    std::ifstream font_file("assets/fonts/Inter_18pt-Medium.ttf");
    if (font_file.is_open()) {
        font_file.close();
        font = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter_18pt-Medium.ttf", 18.0f, NULL, ranges);
    }

    if (font == nullptr) {
        std::cout << "[INFO] Fontul nu a fost gasit pe disk. Se incarca varianta din memoria interna..." << std::endl;
        ImFontConfig font_cfg;

        font_cfg.FontDataOwnedByAtlas = false; 
        
        font = io.Fonts->AddFontFromMemoryTTF((void*)Inter_18pt_Medium, 343200, 18.0f, &font_cfg, ranges);
        
        if (font == nullptr) {
            io.Fonts->AddFontDefault();
        }
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

void renderMeniu(sistem &S, editor &E){
    // Verificăm dacă bara a putut fi creată
if (ImGui::BeginMainMenuBar()) {
    
    // --- Meniul FILE ---
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) { 
            incarcaScenaInitiala(S);
            E.sincronizeazaMemorie(S);
        }
        
        if (ImGui::MenuItem("Open...", "Ctrl+O")) { 
            auto f = pfd::open_file("Incarca Scena", "", { "Fisiere JSON", "*.json", "Toate Fisierele", "*" });
            if (!f.result().empty()) {
                citesteScenaJSON(S, f.result()[0]);
                E.sincronizeazaMemorie(S); // Sincronizam listele si istoricul pentru noile corpuri
            }
        }
        
        if (ImGui::MenuItem("Save Scene As...", "Ctrl+S")) { 
            auto f = pfd::save_file("Salveaza Scena", "scena_salvata.json", { "Fisiere JSON", "*.json", "Toate Fisierele", "*" });
            std::string result = f.result();
            if (!result.empty()) {
                // Ne asiguram ca fisierul va avea extensia .json
                if (result.length() < 5 || result.substr(result.length() - 5) != ".json") {
                    result += ".json";
                }
                salveazaScenaJSON(S, result);
            }
        }
        
        if (ImGui::MenuItem("Export CSV...", "Ctrl+E")) { 
            std::string fisier_ales = cereLocatieSalvare();
            if (!fisier_ales.empty()) {
                E.fisier_export.flush(); 
                
                std::ifstream src(E.nume_fisier_export, std::ios::binary);
                std::ofstream dst(fisier_ales, std::ios::binary);
                if (src && dst) {
                    dst << src.rdbuf(); 
                }
            }
        }
        
        ImGui::Separator(); // Trage o linie orizontala eleganta
        
        if (ImGui::MenuItem("Exit", "Alt+F4")) { 
            // Aici pui logica de inchidere. 
            // Daca ai acces la pointerul ferestrei, pui:
            // glfwSetWindowShouldClose(window, true);
        }
        
        ImGui::EndMenu(); // Nu uita sa inchizi meniul!
    }

    // --- Meniul SETTINGS ---
    if (ImGui::BeginMenu("Settings")) {
        // Poti lega butoanele direct de un bool ca sa devina bifabile (cu checkmark)
        ImGui::MenuItem("Arata forte", NULL, &E.flag.arata_forte);
        ImGui::MenuItem("Arata energie", NULL, &E.flag.arata_energie);
        ImGui::Separator();
        
        ImGui::EndMenu();
    }

    // --- Meniul VIZUALIZARE ---
    if (ImGui::BeginMenu("View")) {
        if (ImGui::BeginMenu("Themes")) {
            static int tema_curenta = 0;
            
            if (ImGui::MenuItem("Mecanica Dark (Default)", NULL, tema_curenta == 0)) {
                tema_curenta = 0;
                ImGui::StyleColorsDark();
                ImGuiStyle& style = ImGui::GetStyle();
                style.Colors[ImGuiCol_WindowBg]         = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
                style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            }
            if (ImGui::MenuItem("ImGui Dark", NULL, tema_curenta == 1)) {
                tema_curenta = 1;
                ImGui::StyleColorsDark();
            }
            if (ImGui::MenuItem("ImGui Light", NULL, tema_curenta == 2)) {
                tema_curenta = 2;
                ImGui::StyleColorsLight();
            }
            if (ImGui::MenuItem("ImGui Classic", NULL, tema_curenta == 3)) {
                tema_curenta = 3;
                ImGui::StyleColorsClassic();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

}


void renderPanouDeControl(sistem &S, editor &E) {
    ImGuiIO& io = ImGui::GetIO(); // O luam la inceput pentru a o avea disponibila peste tot

    if (E.elementeUI.size() < 2) E.elementeUI.resize(2);
    auto &fantoma_corp = E.elementeUI[0];
    
    ImGui::Begin("Panou de Control Mecanica");
    
    // --- 1. SETARI GENERALE & PLAY/PAUSE ---
    ImGui::Text("Timp simulat: %.3f secunde", E.t);
    
    // Buton dinamic care schimba starea simularii
    if (E.mod_curent == MOD_RULARE) {
        if (ImGui::Button("PAUZA [space] ", ImVec2(-1, 30))) {
            E.mod_curent = MOD_EDITARE; 
        }
    } else {
        if (ImGui::Button("PLAY [space]", ImVec2(-1, 30))) {
            E.mod_curent = MOD_RULARE;
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
    static float culoare_aleasa[3] = { 0.3f, 0.6f, 0.9f };
    
   // --- 2. MENIU ADAUGARE (Disponibil doar cand e pe Pauza) ---
    
    if (E.mod_curent != MOD_RULARE) {
        bool in_adaugare = (E.mod_curent >= MOD_ADAUGARE_CORP);
        
        if (!in_adaugare) {
            ImGui::SeparatorText("Adauga Elemente");
            if (ImGui::Button("Adauga Corp Nou", ImVec2(-1, 30))) E.mod_curent = MOD_ADAUGARE_CORP;
            
            // Fără Combo Box, doar butonul direct care pune Incastrarea ca default
            if (ImGui::Button("Adauga Legatura / Arc", ImVec2(-1, 30))) {
                E.mod_curent = MOD_ADAUGARE_LEGATURA_PAS_1; 
                E.tip_legatura_de_adaugat = 1; // 1 = Incastrare (default)
            }
        } else {
            if (ImGui::Button("Anuleaza Adaugarea", ImVec2(-1, 30))) {
                E.mod_curent = MOD_EDITARE;
                fantoma_corp.activa = false;
                if (E.elementeUI.size() > 1) E.elementeUI[1].activa = false;
            }
            
            ImGui::Spacing();
            ImGui::Indent();
            
           if (E.mod_curent == MOD_ADAUGARE_CORP) {
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
                ImGui::Spacing();
                ImGui::PushButtonRepeat(true);
                if(ImGui::Button(u8"↺")) fantoma_corp.phi += 0.05f;; 
                ImGui::SameLine() ; 
                if(ImGui::Button(u8"↻")) fantoma_corp.phi -= 0.05f;
                ImGui::PopButtonRepeat();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "Click Stanga = Plasare | Click Dreapta = Anulare");
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 1.0f, 1.0f), "Apasa Q / E pentru a roti corpul.");
            } 
            else if (E.mod_curent == MOD_ADAUGARE_ARC_PAS_1 || E.mod_curent == MOD_ADAUGARE_ARC_PAS_2 || 
                     E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_1 || E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_2) {
                
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Tip Legatura:");
                
                // Determinam logic ce optiune sa afisam ca fiind bifata
                int selectie_tip = (E.mod_curent == MOD_ADAUGARE_ARC_PAS_1 || E.mod_curent == MOD_ADAUGARE_ARC_PAS_2) ? 2 : E.tip_legatura_de_adaugat;
                int vechea_selectie = selectie_tip;
                
                // Desenam butoanele radio
                ImGui::RadioButton("Articulatie", &selectie_tip, 0); ImGui::SameLine();
                ImGui::RadioButton("Incastrare", &selectie_tip, 1); ImGui::SameLine();
                ImGui::RadioButton("Arc", &selectie_tip, 2);
                
                // Detectam daca ai dat click pe alta varianta si facem tranzitia
                if (selectie_tip != vechea_selectie) {
                    bool era_pas_1 = (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_1 || E.mod_curent == MOD_ADAUGARE_ARC_PAS_1);
                    if (selectie_tip == 2) { // Trecem pe Arc
                        E.mod_curent = era_pas_1 ? MOD_ADAUGARE_ARC_PAS_1 : MOD_ADAUGARE_ARC_PAS_2;
                    } else {                 // Trecem pe Incastrare/Articulatie
                        E.mod_curent = era_pas_1 ? MOD_ADAUGARE_LEGATURA_PAS_1 : MOD_ADAUGARE_LEGATURA_PAS_2;
                        E.tip_legatura_de_adaugat = selectie_tip;
                    }
                }
                
                ImGui::Separator();
                
                // Afisam UI-ul secundar in functie de ce a ramas selectat
                if (selectie_tip == 2) { // E Arc
                    ImGui::SliderFloat("Lungime repaus (%)", &E.arc_l0_procent, 0.0f, 200.0f, "%.0f%%");
                    ImGui::InputFloat("Constanta (k)", &E.arc_k);
                    ImGui::InputFloat("Amortizare (d)", &E.arc_d);
                    ImGui::Separator();
                    if (E.mod_curent == MOD_ADAUGARE_ARC_PAS_1) {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Pas 1: Click pe primul corp / pe fundal");
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Pas 2: Click pe al doilea corp / pe fundal");
                    }
                } else { // E Legatura normala
                    if (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_1) {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Pas 1: Click pe corpul care va fi legat");
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Pas 2: Muta fantoma si click pentru a forma legatura!");
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Apasa Q / E pentru a roti corpul.");
                    }
                }
            }
            
            ImGui::Unindent();
        }
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Opreste simularea pentru a adauga elemente.");
    }
    
    ImGui::End(); // Panoul s-a inchis aici
    
    // --- 3. LOGICA FANTOMEI ---
    
   
    if (E.mod_curent >= MOD_ADAUGARE_CORP) {    
        if (!io.WantCaptureMouse) {
            fantoma_corp.activa = false;
            if (E.elementeUI.size() < 2) E.elementeUI.resize(2);
            auto& fantoma_leg = E.elementeUI[1];
            fantoma_leg.activa = false;
            
             if (E.mod_curent == MOD_ADAUGARE_CORP) {
                fantoma_corp.tip = (forma_selectata == 0) ? 1 : 2;
                fantoma_corp.dim1 = dimensiuni[0]; 
                fantoma_corp.dim2 = (forma_selectata == 0) ? dimensiuni[0] : dimensiuni[1];
                fantoma_corp.activa = true;
                fantoma_corp.x = E.mouse_x;
                fantoma_corp.y = E.mouse_y;
                
                if (ImGui::IsKeyDown(ImGuiKey_Q)) fantoma_corp.phi += 0.05f;
                if (ImGui::IsKeyDown(ImGuiKey_E)) fantoma_corp.phi -= 0.05f;
                
                bool se_suprapune = false;
                
                float fRazaX, fRazaY;
                if (fantoma_corp.tip == 1) { // Cerc
                    fRazaX = fRazaY = fantoma_corp.dim1;
                } else { // Dreptunghi
                    fRazaX = std::abs(std::cos(fantoma_corp.phi) * fantoma_corp.dim1/2.0f) + std::abs(std::sin(fantoma_corp.phi) * fantoma_corp.dim2/2.0f);
                    fRazaY = std::abs(std::sin(fantoma_corp.phi) * fantoma_corp.dim1/2.0f) + std::abs(std::cos(fantoma_corp.phi) * fantoma_corp.dim2/2.0f);
                }
                
                for (auto &corp : S.corpuri) {
                    if (!corp.activ) continue;
                    if (corp.M > 1e10f) continue;
                    if (corp.collider.cadru == E.cadru_activ) {
                        float dist_x = std::abs(fantoma_corp.x - corp.pozitie.x);
                        float dist_y = std::abs(fantoma_corp.y - corp.pozitie.y);
                        if (dist_x < (fRazaX + corp.collider.bb.razaLatime) && 
                        dist_y < (fRazaY + corp.collider.bb.razaInaltime)) {
                            se_suprapune = true;
                            break;
                        }
                    }
                }
                
                if (se_suprapune) {
                    fantoma_corp.col = { 1.0f, 0.0f, 0.0f, 0.6f };
                    ImGui::SetNextWindowPos(ImVec2(io.MousePos.x + 20, io.MousePos.y + 20));
                    ImGui::Begin("TooltipSuprapunere", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Nu poti suprapune corpurile!");
                    ImGui::End();
                } else {
                    fantoma_corp.col = { culoare_aleasa[0], culoare_aleasa[1], culoare_aleasa[2], 0.5f };
                }
                
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
                    
                    forma_noua.seteazaBoundingBox();

                    S.adaugaCorpuri(forma_noua);
                    S.actualizeazaMatriceFizica();
                    E.sincronizeazaMemorie(S);
                }
            }
            else if (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_1 || E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_2) {
                
                // Fantoma corpului agățat apare DOAR în pasul 2 (după ce ai ales corpul)
                if (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_2) {
                    if (E.adaugare_corp_A != 0) {
                        rigid& corpA = S.corpuri[E.adaugare_corp_A];
                        fantoma_corp.tip = corpA.collider.tip;
                        fantoma_corp.dim1 = corpA.collider.dimensiune1;
                        fantoma_corp.dim2 = corpA.collider.dimensiune2;
                        fantoma_corp.col = { corpA.collider.culoare.r, corpA.collider.culoare.g, corpA.collider.culoare.b, 0.5f };
                        fantoma_corp.activa = true;

                        if (ImGui::IsKeyDown(ImGuiKey_Q)) fantoma_corp.phi += 0.05f;
                        if (ImGui::IsKeyDown(ImGuiKey_E)) fantoma_corp.phi -= 0.05f;
                        
                        vec2 offset_rotit(
                            E.adaugare_punct_A_local.x * cos(fantoma_corp.phi) - E.adaugare_punct_A_local.y * sin(fantoma_corp.phi),
                            E.adaugare_punct_A_local.x * sin(fantoma_corp.phi) + E.adaugare_punct_A_local.y * cos(fantoma_corp.phi)
                        );

                        fantoma_corp.x = E.mouse_x - offset_rotit.x;
                        fantoma_corp.y = E.mouse_y - offset_rotit.y;
                    } else {
                        fantoma_corp.activa = false;
                    }
                }
                
                // Fantoma legăturii în sine (Cerc sau Pătrat) - Vizibilă din prima secundă (Pas 1 și Pas 2)
                fantoma_leg.activa = true;
                fantoma_leg.x = E.mouse_x;
                fantoma_leg.y = E.mouse_y;
                
                // Aici controlăm dimensiunea. 0.15f arată mult mai curat pe post de "cui" / punct de sudură
                fantoma_leg.dim1 = 0.15f; 
                fantoma_leg.dim2 = 0.15f;
                
                fantoma_leg.phi = (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_2 && E.adaugare_corp_A != 0) ? fantoma_corp.phi : 0.0f;
                fantoma_leg.tip = (E.tip_legatura_de_adaugat == 0) ? 1 : 2; // 1 = Cerc (Articulatie), 2 = Dreptunghi (Incastrare)
                fantoma_leg.col = {1.0f, 1.0f, 1.0f, 0.8f};
            }
        } else {
            fantoma_corp.activa = false;
            if (E.elementeUI.size() > 1) E.elementeUI[1].activa = false;
        }

        // Anulare manuala
        if (ImGui::IsMouseClicked(1) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            E.mod_curent = MOD_EDITARE;
            fantoma_corp.activa = false;
            if (E.elementeUI.size() > 1) E.elementeUI[1].activa = false;
        }
    } else {
        fantoma_corp.activa = false; 
        if (E.elementeUI.size() > 1) E.elementeUI[1].activa = false;
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
    
    for(int i = 0; i < (int) E.corpuriSelectate.size(); i++  ){
        int id_corp = E.corpuriSelectate[i];
        rigid &r = S.corpuri[id_corp];
        
        ImGui::PushID(id_corp);
        
        if (r.M > 1e10f) {
            ImGui::Text("Tip: Lumea (Element Fix)");
            ImGui::Text("Pozitie: %.2f, %.2f", r.pozitie.x, r.pozitie.y);
        } else {
            
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),"Corp ID: %d | Tip: %s", id_corp,  (r.collider.tip == 1) ? "Cerc" : "Dreptunghi");
            if (ImGui::DragFloat2("Pozitie (X,Y)", &r.pozitie.x, 0.05f)) trebuie_update = true;
            
            float unghi_grade = r.phi * (180.0f / M_PI);
            if (ImGui::DragFloat("Rotatie (°)", &unghi_grade, 0.05f)) {
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
    for (size_t i = 0; i < E.corpuriSelectate.size(); i++) {
        rigid &r = S.corpuri[E.corpuriSelectate[i]];
        if(r.pozitie.x < min_x) min_x = r.pozitie.x;
        if(r.pozitie.x > max_x) max_x = r.pozitie.x;
        if(r.pozitie.y < min_y) min_y = r.pozitie.y;
        if(r.pozitie.y > max_y) max_y = r.pozitie.y;
    }
    vec2 centru_grup((min_x + max_x) / 2.0f, (min_y + max_y) / 2.0f);
    
    float extensie_maxima = 0.5f;
    for( size_t i = 0; i < E.corpuriSelectate.size(); i++){
        rigid &r  = S.corpuri[E.corpuriSelectate[i]];
        
        vec2 offset_fata_de_centru = r.pozitie - centru_grup;
        float dist_la_centru_corp = offset_fata_de_centru.modul();
        
        float ext_geometrica = (r.collider.tip == 1) ? r.collider.dimensiune1 : std::sqrt(std::pow(r.collider.dimensiune1/2.0f, 2) + std::pow(r.collider.dimensiune2/2.0f, 2));
        
        if (dist_la_centru_corp + ext_geometrica > extensie_maxima) {
            extensie_maxima = dist_la_centru_corp + ext_geometrica;
        }
        
        for(size_t j = 0; j < r.forte_desen.forte.size(); j++) {
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
    
    for (int id_corp = 0; id_corp < (int) E.corpuriSelectate.size(); id_corp++) {
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
        for(int j = 0; j < (int) r.forte_desen.forte.size(); j++) {
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

        for(int i = 0; i < (int) E.corpuriSelectate.size(); i++){
            int id_corp = E.corpuriSelectate[i];
            
            if (( (int) E.valoriSimulate.size() )<= id_corp || E.valoriSimulate[id_corp].timpAfisat.empty()) continue;
            
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
            
            ImGui::Spacing();
        }
    }
        //butonul de stergere
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); 
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        
        if (ImGui::Button(E.corpuriSelectate.size() == 1 ? "Elimina Corp" : "Elimina Toate Corpurile", ImVec2(-1, 30))) {
            for (int i = 0; i < (int) E.corpuriSelectate.size(); i++) {
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
        E.vertexBuffer.resize(17 * (S.corpuri.size() + S.legaturi.size() + S.surseForte.size()));

        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        size_t total_elemente =   S.corpuri.size() + S.legaturi.size() + S.surseForte.size();
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
    
    size_t total_elemente = S.corpuri.size() + S.legaturi.size() + S.surseForte.size() + E.elementeUI.size() + nr_forte;

    (E.vertexBuffer).resize(17 * total_elemente);
    drawSystem(S,E, E.VAO, E.VBO, E.shaderProgram, E.vertexBuffer.data());

    renderMeniu(S,E);
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