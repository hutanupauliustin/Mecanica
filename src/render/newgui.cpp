#include "newgui.h"

void setupFont(ImGuiIO& io){
    ImFont* font = nullptr;
    
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

    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x2100, 0x218F, // Letterlike Symbols + Number Forms
        0x2190, 0x21FF, // Arrows 
        0,
    };
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

    setupFont(io);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void startFrameGUI(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

}

void renderMeniu(sistem &S, editor &E){

    if(ImGui::BeginMainMenuBar()){

        // FILE
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) { 
                incarcaScenaInitiala(S);
                E.sincronizeazaMemorie(S);
        }

        if (ImGui::MenuItem("Open...", "Ctrl+O")) { 
            auto f = pfd::open_file("Incarca Scena", "", { "Fisiere JSON", "*.json", "Toate Fisierele", "*" });
            if (!f.result().empty()) {
                citesteScenaJSON(S, f.result()[0]);
                E.sincronizeazaMemorie(S); 
            }
        }




    }

}