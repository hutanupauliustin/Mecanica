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

/*void renderPanouDeAdaugatCorpuri(sistem &S) {
    ImGui::Begin("Panou de Adaugat Corpuri");
    ImGui::Text("Timp simulat: %.3f secunde", t);
    ImGui::Separator();
    ImGui::Checkbox("Ruleaza simularea", &running_flag);
    ImGui::SliderFloat("Pas de timp (dt)", &dt, 0.0001f, 0.01f, "%.4f");
    ImGui::Checkbox("Afiseaza energie", &arata_energie_flag);
    if (arata_energie_flag) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Energie: %.3f KJ", energie / 1000.0f);
    }
    ImGui::End();
}*/

void endFrameGUI(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void cleanupGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}