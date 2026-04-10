#include "newgui.h"
#include "instrument.h"

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

void renderToarePanourile(sistem &S, editor &E){
    renderMeniu(S, E);
    renderOverlayStatus(S, E);
    renderPanouInstrumente(S, E);

    if (!E.corpuriSelectate.empty() || !E.legaturiSelectate.empty()) {
        renderInspector(S, E);
    }
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
}

void renderPanouInstrumente(sistem &S, editor &E) {
    ImGui::Begin("Instrumente");


    if (E.stare_curenta == MOD_RULARE) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Opreste simularea pentru a edita.");
        ImGui::End();
        return; 
    }

    ImGui::Text("Alege o unealta:");
    ImGui::Separator();

    bool eSelectie    = dynamic_cast<InstrumentSelectie*>(E.instrumentCurent.get()) != nullptr;
    bool eCorp        = dynamic_cast<InstrumentAdaugareCorp*>(E.instrumentCurent.get()) != nullptr;
    bool eArc         = dynamic_cast<InstrumentAdaugaArc*>(E.instrumentCurent.get()) != nullptr;
    bool eArticulatie = dynamic_cast<InstrumentAdaugaArticulatie*>(E.instrumentCurent.get()) != nullptr;
    bool eIncastrare  = dynamic_cast<InstrumentAdaugareIncastrare*>(E.instrumentCurent.get()) != nullptr;

    auto deseneazaButon = [](const char* label, bool activ) -> bool {
        if (activ) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.0f)); 
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
        }
        
        bool apasat = ImGui::Button(label, ImVec2(-1, 35)); // -1 face butonul cat toata latimea
        
        if (activ) ImGui::PopStyleColor(3); // Resetăm culorile la normal
        
        return apasat;
    };

    // --- 5. LOGICA BUTOANELOR --- 

    if (deseneazaButon("Cursor (Selectie & Mutare)", eSelectie)) {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }

    ImGui::Spacing(); ImGui::Spacing();
    ImGui::Text("Adaugare Elemente:");
    ImGui::Separator();

    if (deseneazaButon("Adauga Corp", eCorp)) {
        E.schimbaInstrumentCurent(new InstrumentAdaugareCorp());
    }
    
    if (deseneazaButon("Adauga Arc", eArc)) {
        E.schimbaInstrumentCurent(new InstrumentAdaugaArc());
    }
    
    if (deseneazaButon("Adauga Articulatie", eArticulatie)) {
        E.schimbaInstrumentCurent(new InstrumentAdaugaArticulatie());
    }
    
    if (deseneazaButon("Adauga Incastrare", eIncastrare)) {
        E.schimbaInstrumentCurent(new InstrumentAdaugareIncastrare());
    }

    // Aici ai putea adăuga setările secundare ale uneltei (pasul 3)
    // Ex: Dacă eArc e activ și pas == 2, desenezi sliderele de rigiditate/amortizare.

    ImGui::End();
}