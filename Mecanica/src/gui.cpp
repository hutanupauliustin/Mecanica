#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "grafica.h"

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

void renderPanouDeControl(float &dt, bool &running_flag, bool &arata_energie_flag, float t, float energie) {
    ImGui::Begin("Panou de Control Mecanica");
    ImGui::Text("Timp simulat: %.3f secunde", t);
    ImGui::Separator();
    ImGui::Checkbox("Ruleaza simularea", &running_flag);
    ImGui::SliderFloat("Pas de timp (dt)", &dt, 0.0001f, 0.01f, "%.4f");
    ImGui::Checkbox("Afiseaza energie", &arata_energie_flag);
    if (arata_energie_flag) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Energie: %.3f KJ", energie / 1000.0f);
    }
    ImGui::End();
}

void renderPanouDeAdaugatCorpuri(sistem &S) {
    // Ne asiguram ca vectorul este pregatit pentru cele 2 fantome principale
    if (S.elementeUI.size() < 2) {
        S.elementeUI.resize(2);
    }
    
    // Cream referinte pentru a pastra codul usor de citit (0 = Corp, 1 = Legatura)
    auto &fantoma_corp = S.elementeUI[0];
    auto &fantoma_legatura = S.elementeUI[1];

    static float masa = 5.0f;
    static float dimensiuni[2] = { 1.0f, 1.0f };
    static int forma_selectata = 0; 
    ImGuiIO& io = ImGui::GetIO();

    // 1. PLASARE CORP NOU
    if (fantoma_corp.activa) {
        fantoma_corp.x = S.mouse_x;
        fantoma_corp.y = S.mouse_y;

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
            forma_noua.collider.cadru = S.cadru_activ;
            
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
    ImGui::RadioButton("Interactiune", &S.mod_curent, 0); ImGui::SameLine();
    ImGui::RadioButton("Editare", &S.mod_curent, 1);

    if (S.mod_curent == 1) {
        ImGui::Separator();
        ImGui::InputInt("Layer Activ", &S.cadru_activ);
        if (S.cadru_activ < 0) S.cadru_activ = 0;

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
         
        

       /* // 2. PLASARE LEGATURI (In 2 Pasi)
        ImGui::Separator();
        ImGui::Text("Adauga Legaturi:");
        
        if (ImGui::Button("Pioneza (Articulatie)")) { S.stare_legatura = 1; S.legatura_corp_A = -1; }
        ImGui::SameLine();
        if (ImGui::Button("Sudura (Incastrare)")) { S.stare_legatura = 2; S.legatura_corp_A = -1; }

        if (S.stare_legatura != 0) {
            fantoma_legatura.activa = true;
            if (S.stare_legatura == 1) {
                fantoma_legatura.tip = 1; fantoma_legatura.dim1 = 0.15f; 
                fantoma_legatura.col = {0.0f, 1.0f, 1.0f, 1.0f}; 
            } else {
                fantoma_legatura.tip = 2; fantoma_legatura.dim1 = 0.2f; fantoma_legatura.dim2 = 0.2f; 
                fantoma_legatura.col = {1.0f, 0.2f, 0.2f, 1.0f}; 
            }

            if (S.legatura_corp_A == -1) ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PAS 1: Click pe primul corp.");
            else ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "PAS 2: Ancora pe al doilea corp.");

            if (ImGui::IsMouseClicked(0) && !io.WantCaptureMouse) {
                int id_hovered = (S.corpuriSelectate.size() > 0) ? S.corpuriSelectate[0] : 0;
                if (S.legatura_corp_A == -1) {
                    S.legatura_corp_A = id_hovered;
                } else {
                    float mx = fantoma_legatura.x; float my = fantoma_legatura.y;
                    if (S.stare_legatura == 1) S.adaugaLegaturi(articulatie::Creaza(S.corpuri[S.legatura_corp_A], S.corpuri[id_hovered], mx, my));
                    else if (S.stare_legatura == 2) S.adaugaLegaturi(incastrare::Creaza(S.corpuri[S.legatura_corp_A], S.corpuri[id_hovered], mx, my));
                    
                    S.incarcaStare(); S.seteazaJacobian(); S.seteazaConstrangeri();
                    S.legatura_corp_A = -1; 
                }
            } else if (ImGui::IsMouseClicked(1) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                S.stare_legatura = 0; S.legatura_corp_A = -1;
            }
        } else {
            fantoma_legatura.activa = false;
        }

        if (panou_blocat) ImGui::EndDisabled();
    }*/
    ImGui::End();
}
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