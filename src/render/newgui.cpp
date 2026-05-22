#include "newgui.h"
#include "instrument.h"
#include "implot.h"

void setupFont(ImGuiIO &io)
{
    ImFont *font = nullptr;

    static const ImWchar ranges[] = {
        0x0020,
        0x00FF, // Basic Latin + Latin Supplement
        0x2000,
        0x206F, // General Punctuation
        0x2100,
        0x218F, // Letterlike Symbols + Number Forms
        0x2190,
        0x21FF, // Arrows
        0,
    };

    std::ifstream font_file("assets/fonts/Inter_18pt-Medium.ttf");
    if (font_file.is_open())
    {
        font_file.close();

        font = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter_18pt-Medium.ttf", 18.0f, NULL, ranges);
    }

    if (font == nullptr)
    {
        std::cout << "[INFO] Fontul nu a fost gasit pe disk. Se incarca varianta din memoria interna..." << std::endl;
        ImFontConfig font_cfg;

        font_cfg.FontDataOwnedByAtlas = false;

        font = io.Fonts->AddFontFromMemoryTTF((void *)Inter_18pt_Medium, 343200, 18.0f, &font_cfg, ranges);

        if (font == nullptr)
        {
            io.Fonts->AddFontDefault();
        }
    }
}

void setupGUI(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImPlot::CreateContext();
    ImGui::CreateContext();
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f); // Portocaliu
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.FrameRounding = 3.0f;
    style.WindowRounding = 6.0f;
    ImGuiIO &io = ImGui::GetIO();

    setupFont(io);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void startFrameGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void endFrameGUI()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void renderToatePanourile(sistem &S, editor &E)
{
    renderMeniu(S, E);
    renderOverlayStatus(S, E);
    renderPanouInstrumente(S, E);
    E.instrumentCurent->randeazaPanouAditional(E, S);

    if (!E.elementeSelectate.empty())
    {
        renderInspector(S, E);
    }
}

void renderMeniu(sistem &S, editor &E)
{

    if (ImGui::BeginMainMenuBar())
    {

        // FILE
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                incarcaScenaInitiala(S);
                E.sincronizeazaMemorie(S);
            }

            if (ImGui::MenuItem("Open..."))
            {
                auto f = pfd::open_file("Incarca Scena", "", {"Fisiere JSON", "*.json", "Toate Fisierele", "*"});

                if (!f.result().empty())
                {
                    citesteScenaJSON(S, f.result()[0]);
                    E.sincronizeazaMemorie(S);
                }
            }

            if (ImGui::MenuItem("Save..."))
            {
                auto f = pfd::save_file("Salveaza Scena", "", {"Fisiere JSON", "*.json", "Toate Fisierele", "*"});

                if (!f.result().empty())
                {
                    E.sincronizeazaMemorie(S);
                    salveazaScenaJSON(S,f.result());   
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Salveaza log corpuri..."))
            {
                E.flag.salveaza_log_corpuri_la_final = 1;
                E.salveazaLogCorpuri();
            
            }
            if (ImGui::MenuItem("Salveaza log legaturi..."))
            {
                E.flag.salveaza_log_legaturi_la_final = 1;
                E.salveazaLogLegaturi();
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Options")){

            ImGui::MenuItem("Arata Grid", nullptr, &E.flag.arata_grid);
            ImGui::MenuItem("Arata Forte", nullptr, &E.flag.arata_forte);
            //ImGui::MenuItem("Arata Energie", nullptr, &E.flag.arata_energie);
            //ImGui::MenuItem("Arata Grafic", nullptr, &E.flag.arata_forte);

            ImGui::Separator();
            ImGui::TextDisabled("Mod Vizualizare:");

            if (ImGui::MenuItem("Afiseaza Normal", nullptr, E.flag.mod_vizualizare == 0))
                E.flag.mod_vizualizare = 0;
            if (ImGui::MenuItem("Distributie Viteze", nullptr, E.flag.mod_vizualizare == 1))
                E.flag.mod_vizualizare = 1;
            if (ImGui::MenuItem("Distributie Acceleratii", nullptr, E.flag.mod_vizualizare ==2))
                E.flag.mod_vizualizare = 2;

            ImGui::EndMenu();

        }

        if(ImGui::BeginMenu("Style")){

            if (ImGui::BeginMenu("Tema Interfata")) {
                if( ImGui::MenuItem("Default"))
                {
                    ImGuiStyle &style = ImGui::GetStyle();
                    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
                    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                    style.FrameRounding = 3.0f;
                    style.WindowRounding = 6.0f;
                }

                if (ImGui::MenuItem("Dark ")) ImGui::StyleColorsDark();
                if (ImGui::MenuItem("Light")) ImGui::StyleColorsLight();
                if (ImGui::MenuItem("Classic")) ImGui::StyleColorsClassic();
            
                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu("Modifica Culoarea de Fundal")){
                ImGui::TextDisabled("Alege culoarea spatiului:");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::ColorEdit3("##fundal", E.flag.culoare_fundal, ImGuiColorEditFlags_NoInputs);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Reseteaza")) {
                    E.flag.culoare_fundal[0] = 0.1f;
                    E.flag.culoare_fundal[1] = 0.1f;
                    E.flag.culoare_fundal[2] = 0.1f;
                }
                ImGui::EndMenu();
            } 
            ImGui::EndMenu(); 
        }
        ImGui::EndMainMenuBar();
    }
}

void renderInspector(sistem &S, editor &E)
{
    ImGui::Begin("Inspector");

    if (!E.elementeSelectate.empty())
    {
        ImGui::Text("Obiecte selectate: %d", (int)E.elementeSelectate.size());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

        if (ImGui::Button("Sterge Selectia", ImVec2(-1, 35)))
        {

            for (const auto &el : E.elementeSelectate)
            {
                if (el.tip == TIP_CORP)
                    S.eliminaCorp(el.id);
                else if (el.tip == TIP_LEGATURA)
                    S.eliminaLegatura(el.id);
            }

            E.elementeSelectate.clear();
            S.actualizeazaMatriceFizica();
        }
        ImGui::PopStyleColor(3);

        for(int i = 0; i < E.elementeSelectate.size(); i++){
            
            auto &el = E.elementeSelectate[i];

                    // --- 1. EDITARE CORPURI ---
            if (el.tip == TIP_CORP && el.id < S.corpuri.size()) {
                rigid &r = S.corpuri[el.id];

                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Proprietati Corp [#%d]", el.id);

                // Parametri Fizici
                if (ImGui::CollapsingHeader("Fizica", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat2("Pozitie", &r.pozitie.x, 0.05f);
                    float unghi_grade = r.phi * 180.0f / 3.1415f;
                    if (ImGui::DragFloat("Rotatie (deg)", &unghi_grade, 1.0f)) {
                        r.phi = unghi_grade * 3.1415f / 180.0f;
                    }
                    ImGui::DragFloat("Masa (kg)", &r.M, 0.1f, 0.001f, 10000.0f);
                }
            
                // Parametri Vizuali (RGBA)
                if (ImGui::CollapsingHeader("Aspect", ImGuiTreeNodeFlags_DefaultOpen)) {
                    // Mapping direct pe structura ta de culoare
                    float col[4] = { r.collider.culoare.r, r.collider.culoare.g, r.collider.culoare.b, r.collider.culoare.a };
                    if (ImGui::ColorEdit4("Culoare Corp", col)) {
                        r.collider.culoare = { col[0], col[1], col[2], col[3] };
                    }
                }
            }
            // --- 2. EDITARE LEGATURI ---
            else if (el.tip == TIP_LEGATURA && el.id < S.legaturi.size()) {
                legatura* l = S.legaturi[el.id];
                ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f), "Proprietati Legatura [#%d]", el.id);

                float colL[4] = { l->culoare.x, l->culoare.y, l->culoare.z, l->culoare.w };
                if (ImGui::ColorEdit4("Culoare Linie", colL)) {
                    l->culoare = { colL[0], colL[1], colL[2], colL[3] };
                }
            
                // Parametri specifici in functie de tip
                if (fir* f = dynamic_cast<fir*>(l)) {
                    ImGui::DragFloat("Lungime Fir", &f->lungime, 0.05f, 0.1f, 100.0f);
                }
                else if (incastrare* inc = dynamic_cast<incastrare*>(l)) {
                    float unghi = inc->phi_0 * 180.0f / 3.1415f;
                    if (ImGui::DragFloat("Unghi Blocat", &unghi, 1.0f)) {
                        // Va trebui sa ai un setPhi0 daca vrei sa-l modifici live
                    }
                }
            }
            // --- 3. EDITARE GENERATORI DE FORTE ---
            else if (el.tip == TIP_GENERATOR_FORTA && el.id < S.surseForte.size()) {
                auto* gen = S.surseForte[el.id];
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.7f, 1.0f), "Sursa de Forta [#%d]", el.id);
            
                float colG[4] = { gen->culoare.x, gen->culoare.y, gen->culoare.z, gen->culoare.w };
                if (ImGui::ColorEdit4("Culoare Forta", colG)) {
                    gen->culoare = { colG[0], colG[1], colG[2], colG[3] };
                }

                // Aici poti adauga si magnitudinea fortei daca ai acces la ea
            }
            ImGui::NewLine();

        
            ImGui::Spacing();
            ImGui::Separator();
            if (el.tip == TIP_LEGATURA && el.id >= 0 && el.id < E.valoriLegaturi.size())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("Reactiuni Legatura");

                IstoricLegatura &ist = E.valoriLegaturi[el.id];
                int nr_puncte = ist.timpAfisat.size();

                if (nr_puncte > 0)
                {
                    if (ImPlot::BeginPlot("##GraficLegatura", ImVec2(-1, 200)))
                    {
                        ImPlot::SetupAxes("Timp (s)", "Valoare (N, Nm)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

                        ImPlotSpec spec;
                        spec.Offset = ist.offset;

                        ImPlot::PlotLine("Reactiune X (N)", ist.timpAfisat.data(), ist.fortaX.data(), nr_puncte, spec);
                        ImPlot::PlotLine("Reactiune Y (N)", ist.timpAfisat.data(), ist.fortaY.data(), nr_puncte, spec);
                        ImPlot::PlotLine("Moment (Nm)", ist.timpAfisat.data(), ist.moment.data(), nr_puncte, spec);

                        ImPlot::EndPlot();
                    }
                }
                else
                {
                    ImGui::TextDisabled("Nu exista date. Ruleaza simularea.");
                }
            }
            else if (el.tip == TIP_CORP && el.id > 0 && el.id < E.valoriSimulate.size()) // el.id > 0 pentru a exclude corpul "Lume"
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("Grafic Parametri Corp");

                IstoricCorp &ist = E.valoriSimulate[el.id];
                int nr_puncte = ist.timpAfisat.size();

                if (nr_puncte > 0)
                {
                    if (ImPlot::BeginPlot("##GraficCorp", ImVec2(-1, 200)))
                    {
                        ImPlot::SetupAxes("Timp (s)", "Valoare", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

                        ImPlotSpec spec;
                        spec.Offset = ist.offset;

                        ImPlot::PlotLine("Pozitie X", ist.timpAfisat.data(), ist.axe[POZITIE_X].data(), nr_puncte, spec);
                        ImPlot::PlotLine("Pozitie Y", ist.timpAfisat.data(), ist.axe[POZITIE_Y].data(), nr_puncte, spec);
                        ImPlot::PlotLine("Unghi Phi", ist.timpAfisat.data(), ist.axe[POZITIE_PHI].data(), nr_puncte, spec);
                        
                        ImPlot::PlotLine("Viteza X", ist.timpAfisat.data(), ist.axe[VITEZA_X].data(), nr_puncte, spec);
                        ImPlot::PlotLine("Viteza Y", ist.timpAfisat.data(), ist.axe[VITEZA_Y].data(), nr_puncte, spec);
                        ImPlot::PlotLine("Viteza Ungh.", ist.timpAfisat.data(), ist.axe[VITEZA_OMEGA].data(), nr_puncte, spec);
                        
                        ImPlot::PlotLine("Acc. X", ist.timpAfisat.data(), ist.axe[ACCELERATIE_X].data(), nr_puncte, spec);
                        ImPlot::PlotLine("Acc. Y", ist.timpAfisat.data(), ist.axe[ACCELERATIE_Y].data(), nr_puncte, spec);
                        ImPlot::PlotLine("Acc. Ungh.", ist.timpAfisat.data(), ist.axe[ACCELERATIE_EPSILON].data(), nr_puncte, spec);

                        ImPlot::EndPlot();
                    }
                }
                else
                {
                    ImGui::TextDisabled("Nu exista date. Ruleaza simularea.");
                }
            }
        }
    }

    ImGui::End();
}

void renderOverlayStatus(sistem &S, editor &E)
{
}

void renderPanouInstrumente(sistem &S, editor &E)
{
 ImGui::Begin("Instrumente");

    if (E.mod_curent == MOD_RULARE)
    {
        if (ImGui::Button("Pause", ImVec2(80, 0))) {
            E.mod_curent = MOD_EDITARE;
        }
        
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::SliderFloat("Viteza", &S.scala_timp, 0.1f, 5.0f, "%.1fx");

        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Opreste simularea pentru a edita.");
        ImGui::End();
        return;
    }

    if (ImGui::Button("Play", ImVec2(80, 0))) {
        E.mod_curent = MOD_RULARE;
    }
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::SliderFloat("Viteza", &S.scala_timp, 0.1f, 5.0f, "%.1fx");

    ImGui::Text("Alege o unealta:");
    ImGui::Separator();

    bool eSelectie = dynamic_cast<InstrumentSelectie *>(E.instrumentCurent.get()) != nullptr;
    bool eCorp = dynamic_cast<InstrumentAdaugaCorp *>(E.instrumentCurent.get()) != nullptr;
    bool eArc = dynamic_cast<InstrumentAdaugaArc *>(E.instrumentCurent.get()) != nullptr;
    bool eArticulatie = dynamic_cast<InstrumentAdaugaArticulatie *>(E.instrumentCurent.get()) != nullptr;
    bool eIncastrare = dynamic_cast<InstrumentAdaugaIncastrare *>(E.instrumentCurent.get()) != nullptr;
    bool eFir = dynamic_cast<InstrumentAdaugaFir *>(E.instrumentCurent.get()) != nullptr;
    bool eMotor = dynamic_cast<InstrumentAdaugaMotor *>(E.instrumentCurent.get()) != nullptr;

    auto deseneazaButon = [](const char *label, bool activ) -> bool
    {
        if (activ)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
        }

        bool apasat = ImGui::Button(label, ImVec2(-1, 35)); // -1 face butonul cat toata latimea

        if (activ)
            ImGui::PopStyleColor(3); // Resetăm culorile la normal

        return apasat;
    };

    if (deseneazaButon("Cursor (Selectie & Mutare)", eSelectie))
    {
        E.schimbaInstrumentCurent(new InstrumentSelectie());
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("Adaugare Elemente:");
    ImGui::Separator();

    if (deseneazaButon("Adauga Corp", eCorp))
    {
        E.schimbaInstrumentCurent(new InstrumentAdaugaCorp());
    }

    if (deseneazaButon("Adauga Arc", eArc))
    {
        E.schimbaInstrumentCurent(new InstrumentAdaugaArc());
    }

    if (deseneazaButon("Adauga Articulatie", eArticulatie))
    {
        E.schimbaInstrumentCurent(new InstrumentAdaugaArticulatie());
    }

    if (deseneazaButon("Adauga Incastrare", eIncastrare))
    {
        E.schimbaInstrumentCurent(new InstrumentAdaugaIncastrare());
    }

    /*if (deseneazaButon("Adauga Fir", eFir))
    {
        E.schimbaInstrumentCurent(new InstrumentAdaugaFir());
    }*/

    /*if(deseneazaButon("Adauga Motor", eMotor))
    {
        E.schimbaInstrumentCurent(new InstrumentAdaugaMotor());
    }*/



    // Aici ai putea adăuga setările secundare ale uneltei (pasul 3)
    // Ex: Dacă eArc e activ și pas == 2, desenezi sliderele de rigiditate/amortizare.

    if (E.instrumentCurent != nullptr)
    {
        E.instrumentCurent->deseneazaSetariUI(E, S);
    }

    ImGui::End();
}

GLFWwindow *initializareGrafica(sistem &S, editor &E)
{

    std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
    GLFWwindow *window = openGLWindow(E.shaderProgram, E.gridShaderProgram);

    if (window != NULL)
    {

        glfwSetWindowUserPointer(window, &E);
        initBuffers(E.VAO, E.VBO, E.gridVAO, E.gridVBO);

        setupGUI(window);

        E.vertexBuffer.resize(17 * (S.corpuri.size() + S.legaturi.size() + S.surseForte.size()));

        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        size_t total_elemente = S.corpuri.size() + S.legaturi.size() + S.surseForte.size();
        E.vertexBuffer.resize(E.vertexStride * total_elemente);
    }
    return window;
}

void randareGrafica(sistem &S, editor &E)
{

    size_t nr_forte = 0;
    if (E.flag.arata_forte)
    {
        for (const auto &corp : S.corpuri)
        {
            if (corp.activ && corp.M < 1e10f)
            {
                nr_forte += corp.forte_desen.forte.size();
            }
        }
    }

    size_t total_elemente = S.corpuri.size() + S.legaturi.size() + S.surseForte.size() + E.elementeUI.size() + nr_forte;

    glClearColor(E.flag.culoare_fundal[0],E.flag.culoare_fundal[1],E.flag.culoare_fundal[2],1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawGrid(E);

    (E.vertexBuffer).resize(17 * total_elemente);
    drawSystem(S, E, E.VAO, E.VBO, E.shaderProgram, E.vertexBuffer.data());

    renderToatePanourile(S, E);
    endFrameGUI();
    glfwSwapBuffers(E.window);
    glfwPollEvents();

    if (E.mod_curent == MOD_RULARE)
        (E.frameCount)++;
    
}

void cleanupGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}
