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
    
    static int id_corp_fantoma = -1;
    static float masa_fantoma = 5.0f;
    
    // Am mutat declaratiile aici sus, inainte de a fi folosite
    static float masa = 5.0f;
    static float dimensiuni[2] = { 1.0f, 1.0f };

    ImGuiIO& io = ImGui::GetIO();

    // 1. Logica de actualizare si plasare a corpului fantoma
    if (id_corp_fantoma != -1 && id_corp_fantoma < (int)S.corpuri.size()) {
        float mx = S.corpuri[S.id_corp_mouse].x;
        float my = S.corpuri[S.id_corp_mouse].y;

        // Urmareste cursorul
        S.corpuri[id_corp_fantoma].x = mx;
        S.corpuri[id_corp_fantoma].y = my;
        S.stare(id_corp_fantoma * 3, 0) = mx;
        S.stare(id_corp_fantoma * 3 + 1, 0) = my;
        S.corpuri[id_corp_fantoma].v_x = 0;
        S.corpuri[id_corp_fantoma].v_y = 0;
        S.corpuri[id_corp_fantoma].omega = 0;

        // Plasare la Click Stanga (in afara ferestrelor ImGui)
        if (ImGui::IsMouseClicked(0) && !io.WantCaptureMouse) {
            std::cout << "[GUI] Click stanga! Plasare corp fantoma definitiv. ID = " << id_corp_fantoma << std::endl;
            S.corpuri[id_corp_fantoma].M = masa_fantoma;
            // Recalculam Inerția pe baza masei reale
            if (S.corpuri[id_corp_fantoma].collider.tip == CERC) { 
                float r = S.corpuri[id_corp_fantoma].collider.dimensiune1;
                S.corpuri[id_corp_fantoma].J = (masa_fantoma * r * r) / 2.0f;
            } else { 
                float l = S.corpuri[id_corp_fantoma].collider.dimensiune1;
                float h = S.corpuri[id_corp_fantoma].collider.dimensiune2;
                S.corpuri[id_corp_fantoma].J = (masa_fantoma * (l * l + h * h)) / 12.0f;
            }
            S.corpuri[id_corp_fantoma].collider.cadru = S.cadru_activ; // Alocăm corpul cadru-ului ales
            S.corpuri[id_corp_fantoma].collider.culoare.a = 1.0f; // Îl facem opac la loc
            S.seteazaMatriceInertie();
            std::cout << "[GUI] Corp plasat cu succes. Inerție si Masa recalculate." << std::endl;
            id_corp_fantoma = -1;
        }
        // Anulare la Click Dreapta sau Escape
        else if (ImGui::IsMouseClicked(1) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            std::cout << "[GUI] Anulare plasare corp fantoma." << std::endl;
            S.corpuri[id_corp_fantoma].activ = 0;
            S.corpuri[id_corp_fantoma].x = -9999.0f; // Il mutam din peisaj (va fi invizibil)
            S.incarcaStare();
            S.seteazaMatriceInertie();
            id_corp_fantoma = -1;
        }
    }

    ImGui::Begin("Panou de Adaugat Corpuri");

    ImGui::Text("Mod de utilizare:");
    ImGui::RadioButton("Interactiune", &S.mod_curent, INTERACTIONARE); ImGui::SameLine();
    ImGui::RadioButton("Editare", &S.mod_curent, EDITARE);

    if (S.mod_curent == EDITARE) {
        ImGui::Separator();
        ImGui::Text("Setari Cadru (Layer):");
        ImGui::InputInt("Layer Activ", &S.cadru_activ);
        if (S.cadru_activ < 0) {
            S.cadru_activ = 0;
        }

        ImGui::InputFloat("Masa", &masa);
        if (masa <= 0.01f) masa = 0.01f; 

        // 1. Salvăm starea inițială a panoului pentru acest cadru
        bool panou_blocat = (id_corp_fantoma != -1);

        // 2. Aplicăm blocajul doar dacă era deja selectată o fantomă la începutul cadrului
        if (panou_blocat) ImGui::BeginDisabled();

        ImGui::Separator();
        ImGui::Text("Adauga Cerc:");
        ImGui::InputFloat("Raza", &dimensiuni[0]);
        if (dimensiuni[0] <= 0.05f) dimensiuni[0] = 0.05f; 
        
        if (ImGui::Button("Selecteaza Cerc")) {
            float mx = S.corpuri[S.id_corp_mouse].x;
            float my = S.corpuri[S.id_corp_mouse].y;
            rigid cerc = rigid::Disc(mx, my, dimensiuni[0], 1e12f); 
            cerc.collider.culoare = {0.3f, 0.9f, 0.3f, 0.4f};       
            cerc.collider.cadru = -1;                               
            S.adaugaCorpuri(cerc);
            S.incarcaStare();           
            S.seteazaMatriceInertie();  
            
            id_corp_fantoma = cerc.index; 
            masa_fantoma = masa;
        }

        ImGui::Separator();
        ImGui::Text("Adauga Dreptunghi:");
        ImGui::InputFloat2("Dimensiuni (L, h)", dimensiuni);
        if (dimensiuni[0] <= 0.05f) dimensiuni[0] = 0.05f; 
        if (dimensiuni[1] <= 0.05f) dimensiuni[1] = 0.05f; 
        
        if (ImGui::Button("Selecteaza Dreptunghi")) {
            float mx = S.corpuri[S.id_corp_mouse].x;
            float my = S.corpuri[S.id_corp_mouse].y;
            rigid drept = rigid::Bara(mx, my, dimensiuni[0], dimensiuni[1], 1e12f);
            drept.collider.culoare = {0.9f, 0.3f, 0.9f, 0.4f}; 
            drept.collider.cadru = -1;                         
            S.adaugaCorpuri(drept);
            S.incarcaStare();
            S.seteazaMatriceInertie();

            id_corp_fantoma = drept.index; 
            masa_fantoma = masa;
        }

        // --- SECTIUNEA PENTRU LEGATURI (Snapping Automat) ---
        ImGui::Separator();
        ImGui::Text("Adauga Legaturi (Ancorare la Lume):");
        
        static int stare_legatura = 0; // 0 = inactiv, 1 = articulatie, 2 = incastrare

        if (ImGui::Button("Pioneza (Articulatie)")) stare_legatura = 1;
        ImGui::SameLine();
        if (ImGui::Button("Sudura (Incastrare)")) stare_legatura = 2;

        if (stare_legatura != 0) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "-> Click pe un corp pentru a-l fixa.");
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "-> Click DREAPTA pt anulare.");

            // Daca dam click si nu suntem cu mouse-ul pe o fereastra ImGui
            if (ImGui::IsMouseClicked(0) && !io.WantCaptureMouse) {
                
                // Snapping automat: Luam corpul aflat sub mouse
                if (S.corpuriSelectate.size() > 0) {
                    int id_corp_B = S.corpuriSelectate[0];
                    
                    if (id_corp_B > 1) { // Ne asiguram ca nu legam Lumea de Lume
                        float mx = S.corpuri[S.id_corp_mouse].x;
                        float my = S.corpuri[S.id_corp_mouse].y;

                        // Corp A e mereu Lumea (0) pentru ancorare
                        if (stare_legatura == 1) {
                            S.adaugaLegaturi(articulatie::Creaza(S.corpuri[0], S.corpuri[id_corp_B], mx, my));
                        } else if (stare_legatura == 2) {
                            S.adaugaLegaturi(incastrare::Creaza(S.corpuri[0], S.corpuri[id_corp_B], mx, my));
                        }

                        // Reinitializam matricele ca sa includa noile constrangeri
                        S.incarcaStare();
                        S.seteazaJacobian();
                        S.seteazaConstrangeri();
                    }
                }
                stare_legatura = 0; // Se reseteaza dupa plasare
            }
            // Anulam cu click dreapta
            else if (ImGui::IsMouseClicked(1) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                stare_legatura = 0;
            }
        }

        // 3. Închidem blocajul folosind ACEEAȘI variabilă de la început
        if (panou_blocat) {
            ImGui::EndDisabled();
        }

        // Textul informativ îl putem afișa folosind variabila actualizată (vrem să apară instant)
        if (id_corp_fantoma != -1) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "-> Click STANGA in spatiu pentru a plasa.");
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "-> Click DREAPTA sau ESC pt. anulare.");
        }

    } // Aici se inchide blocul if (S.mod_curent == 1)
    
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