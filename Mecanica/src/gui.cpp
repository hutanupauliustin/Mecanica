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
    ImGui::Begin("Panou de Adaugat Corpuri");
    
    static float pos[2] = { 0.0f, 10.0f }; // Corpurile vor cadea de la inaltimea Y=10
    static float masa = 5.0f;
    static float dimensiuni[2] = { 1.0f, 1.0f }; // Pt cerc folosim doar X (Raza)

    ImGui::InputFloat2("Pozitie (X, Y)", pos);
    ImGui::InputFloat("Masa", &masa);
    if (masa <= 0.01f) masa = 0.01f; // Prevenim mase negative sau zero

    ImGui::Separator();
    ImGui::Text("Adauga Cerc:");
    ImGui::InputFloat("Raza", &dimensiuni[0]);
    if (ImGui::Button("Creeaza Cerc")) {
        rigid cerc = rigid::Disc(pos[0], pos[1], dimensiuni[0], masa);
        cerc.collider.culoare = {0.3f, 0.9f, 0.3f, 1.0f}; // Culoare verde distincta
        S.adaugaCorpuri(cerc);
        S.incarcaStare();           // Redimensioneaza matricea starii
        S.seteazaMatriceInertie();  // Redimensioneaza si recalculeaza inertiile
    }

    ImGui::Separator();
    ImGui::Text("Adauga Dreptunghi:");
    ImGui::InputFloat2("Dimensiuni (L, h)", dimensiuni);
    if (ImGui::Button("Creeaza Dreptunghi")) {
        rigid drept = rigid::Bara(pos[0], pos[1], dimensiuni[0], dimensiuni[1], masa);
        drept.collider.culoare = {0.9f, 0.3f, 0.9f, 1.0f}; // Culoare mov distincta
        S.adaugaCorpuri(drept);
        S.incarcaStare();
        S.seteazaMatriceInertie();
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