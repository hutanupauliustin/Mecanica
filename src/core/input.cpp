#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include "sistem.h"
#include "editor.h"
#include "input.h"
#include "imgui.h"

float zoomScale = 10.0f;
float cameraX = 0.0f;
float cameraY = 0.0f;
float aspect_ratio = 800.0f / 600.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse)
        return;
    zoomScale -= (float)yoffset * 0.5f; 
    
    if (zoomScale < 1.0f) zoomScale = 1.0f; 
    if (zoomScale > 100.0f) zoomScale = 100.0f; 
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    if (height == 0) height = 1; 
    glViewport(0, 0, width, height);
    aspect_ratio = (float)width / (float)height; 
}

/*void processInput(GLFWwindow *window, sistem &S, editor &E) {

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool plusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        if(!plusApasat){
            E.dt += 0.001f; 
            if(E.dt > 0.01f) E.dt = 0.01f;
            plusApasat = true;
        }
    } else {
        plusApasat = false;
    }

    static bool minusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        if(!minusApasat){
            E.dt -= 0.001f; 
            if(E.dt < 0.0001f) E.dt = 0.0001f;
            minusApasat = true;
        }
    } else {
        minusApasat = false;
    }

    static bool spaceApasat = false;

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        if(!spaceApasat) {
           E.mod_curent = E.mod_curent != MOD_RULARE ?  MOD_RULARE : MOD_EDITARE;
            spaceApasat = true;
            std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
        }
    } else {
        spaceApasat = false; 
    }

    //-------Mouse Input------//
      
    ImGuiIO& io = ImGui::GetIO();

    // Ștergerea corpurilor si legaturilor selectate cu Delete sau Backspace
    static bool deleteApasat = false;
    if(glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        if(!deleteApasat && !io.WantCaptureKeyboard && (!E.corpuriSelectate.empty() || !E.legaturiSelectate.empty())) {
            
            for (int id : E.corpuriSelectate) { S.eliminaCorp(id); }
            for (int id : E.legaturiSelectate) { S.eliminaLegatura(id); }

            E.corpuriSelectate.clear();
            E.legaturiSelectate.clear();
            E.sincronizeazaMemorie(S);
            deleteApasat = true;
        }
    } else {
        deleteApasat = false;
    }

    // Ctrl+A pentru a selecta toate corpurile
    static bool ctrlAApasat = false;
    bool hasCtrl = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    if(hasCtrl && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if(!ctrlAApasat && !io.WantCaptureKeyboard) {
            for (size_t i = 1; i < S.corpuri.size(); i++) {
                if (S.corpuri[i].activ && S.corpuri[i].M < 1e10f && std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), i) == E.corpuriSelectate.end()) {
                    E.corpuriSelectate.push_back(i);
                    S.corpuri[i].collider.selectat = 1;
                }
            }
            ctrlAApasat = true;
        }
    } else {
        ctrlAApasat = false;
    }

    static float ndcX_anterior = ndcX;
    static float ndcY_anterior = ndcY;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS && !io.WantCaptureMouse) {
        float delta_camX = (ndcX - ndcX_anterior) * zoomScale * aspect_ratio;
        float delta_camY = (ndcY - ndcY_anterior) * zoomScale;
        
        cameraX -= delta_camX;
        cameraY -= delta_camY;
    }

    ndcX_anterior = ndcX;
    ndcY_anterior = ndcY;

    mouseX = ndcX * zoomScale * aspect_ratio + cameraX;
    mouseY = ndcY * zoomScale + cameraY;

    E.mouse_x = mouseX;
    E.mouse_y = mouseY;

    // --- HOVER LOGIC ---
    for(size_t i = 0; i <  E.corpuriSubMouse.size(); i++)
        S.corpuri[E.corpuriSubMouse[i]].collider.subMouse = 0;
    E.corpuriSubMouse.clear();
    
    int id_sub_mouse = E.gasesteCorpSubMouse(S);
    if (id_sub_mouse != -1) {
        S.corpuri[id_sub_mouse].collider.subMouse = 1;
        E.corpuriSubMouse.push_back(id_sub_mouse);
    }

    // Hover Legaturi
    int id_legatura_sub_mouse = -1;
    float distMinLegatura = 0.5f; // Raza de toleranta pentru click (in metri)
    
    for (size_t i = 0; i < S.legaturi.size(); i++) {
        S.legaturi[i]->subMouse = false; // Resetam hover-ul vechi
        if (!S.legaturi[i]->activ) continue;

        vec2 pozLeg = S.legaturi[i]->getPozitie(S.corpuri);
        float distanta = (vec2(mouseX, mouseY) - pozLeg).modul();

        if (distanta < distMinLegatura) {
            distMinLegatura = distanta; 
            id_legatura_sub_mouse = i;
        }
    }

    if (id_legatura_sub_mouse != -1) {
        S.legaturi[id_legatura_sub_mouse]->subMouse = true;
    }

    static bool stangaApasat = false;
    bool stangaClick = false;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!stangaApasat) {
            stangaClick = true; 
            stangaApasat = true;
        }
    } else {
        stangaApasat = false;
    }

    bool hasShift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // Variabile de stare pentru Box Selection
    static bool isBoxSelecting = false;
    static vec2 boxSelectStartFizic(0.0f, 0.0f);
    static std::vector<int> selectieInitiala;
    static std::vector<int> selectieInitialaLegaturi;

    if(stangaClick && !io.WantCaptureMouse){
        if (E.mod_curent == MOD_ADAUGARE_ARC_PAS_1) {
            E.adaugare_corp_A = (id_sub_mouse != -1) ? id_sub_mouse : 0;
            E.adaugare_punct_A_local = S.corpuri[E.adaugare_corp_A].globalToLocal(vec2(mouseX, mouseY));
            E.mod_curent = MOD_ADAUGARE_ARC_PAS_2;
        }
        else if (E.mod_curent == MOD_ADAUGARE_ARC_PAS_2) {
            E.adaugare_corp_B = (id_sub_mouse != -1) ? id_sub_mouse : 0;
            E.adaugare_punct_B_local = S.corpuri[E.adaugare_corp_B].globalToLocal(vec2(mouseX, mouseY));
            
            vec2 pA_global = S.corpuri[E.adaugare_corp_A].localToGlobal(E.adaugare_punct_A_local);
            vec2 pB_global = S.corpuri[E.adaugare_corp_B].localToGlobal(E.adaugare_punct_B_local);
            float dist = (pA_global - pB_global).modul();
            float l0_real = dist * (E.arc_l0_procent / 100.0f);
            
            arc* arc_nou = arc::Creaza(S.corpuri[E.adaugare_corp_A], S.corpuri[E.adaugare_corp_B],
                                      pA_global.x, pA_global.y, pB_global.x, pB_global.y,
                                      E.arc_k, E.arc_d, l0_real);
            S.adaugaGeneratorForte(arc_nou);
            S.actualizeazaMatriceFizica();
            E.elementeUI[1].activa = false;
            E.mod_curent = MOD_EDITARE;
        }
        else if (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_1) {
            E.adaugare_corp_A = (id_sub_mouse != -1) ? id_sub_mouse : 0;
            E.adaugare_punct_A_local = S.corpuri[E.adaugare_corp_A].globalToLocal(vec2(mouseX, mouseY));
            E.elementeUI[0].phi = S.corpuri[E.adaugare_corp_A].phi; 
            E.mod_curent = MOD_ADAUGARE_LEGATURA_PAS_2;
        }
        else if (E.mod_curent == MOD_ADAUGARE_LEGATURA_PAS_2) {
            E.adaugare_corp_B = (id_sub_mouse != -1 && id_sub_mouse != E.adaugare_corp_A) ? id_sub_mouse : 0;
            
            rigid& corpA = S.corpuri[E.adaugare_corp_A];
            if (E.adaugare_corp_A != 0) {
                corpA.phi = E.elementeUI[0].phi;
                corpA.pozitie.x = E.elementeUI[0].x;
                corpA.pozitie.y = E.elementeUI[0].y;
                corpA.viteza = vec2(0.0f, 0.0f); corpA.omega = 0.0f;
            }

            if (E.tip_legatura_de_adaugat == 0) {
                S.adaugaLegaturi(articulatie::Creaza(corpA, S.corpuri[E.adaugare_corp_B], mouseX, mouseY));
            } else {
                S.adaugaLegaturi(incastrare::Creaza(corpA, S.corpuri[E.adaugare_corp_B], mouseX, mouseY));
            }
            S.actualizeazaMatriceFizica();
            E.elementeUI[0].activa = false;
            if (E.elementeUI.size() > 1) E.elementeUI[1].activa = false;
            E.mod_curent = MOD_EDITARE;
        }
        else if (E.mod_curent == MOD_ADAUGARE_CORP) {
            // Plasarea corpului intregrat in gui.cpp
        }
        else if(id_sub_mouse != -1){
            
            bool dejaSelectat = ( std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), id_sub_mouse) != E.corpuriSelectate.end());
            
            if (!hasShift) {
                // Dacă selectăm un corp fără Shift, deselectăm legăturile anterioare
                for (int idL : E.legaturiSelectate) S.legaturi[idL]->selectat = false;
                E.legaturiSelectate.clear();
            }

            if (hasShift) {
                if (dejaSelectat) {
                    S.corpuri[id_sub_mouse].collider.selectat = 0;
                    E.corpuriSelectate.erase(std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), id_sub_mouse));
                } else {
                    S.corpuri[id_sub_mouse].collider.selectat = 1;
                    E.corpuriSelectate.push_back(id_sub_mouse);
                }
            } else {
                if (!dejaSelectat) { 
                    for (size_t i = 0 ; i < E.corpuriSelectate.size(); i++) {
                        S.corpuri[E.corpuriSelectate[i]].collider.selectat = 0;
                    }
                    E.corpuriSelectate.clear();
                    
                    S.corpuri[id_sub_mouse].collider.selectat = 1;
                    E.corpuriSelectate.push_back(id_sub_mouse);
                }
            }
        
        } 
        else if (id_legatura_sub_mouse != -1) {
            
            bool dejaSelectat = (std::find(E.legaturiSelectate.begin(), E.legaturiSelectate.end(), id_legatura_sub_mouse) != E.legaturiSelectate.end());
            
            if (hasShift) {
                if (dejaSelectat) {
                    S.legaturi[id_legatura_sub_mouse]->selectat = false;
                    E.legaturiSelectate.erase(std::remove(E.legaturiSelectate.begin(), E.legaturiSelectate.end(), id_legatura_sub_mouse), E.legaturiSelectate.end());
                } else {
                    S.legaturi[id_legatura_sub_mouse]->selectat = true;
                    E.legaturiSelectate.push_back(id_legatura_sub_mouse);
                }
            } else {
                if (!dejaSelectat) { 
                    // Curatam corpurile
                    for (int idC : E.corpuriSelectate) S.corpuri[idC].collider.selectat = 0;
                    E.corpuriSelectate.clear();
                    
                    // Curatam legaturile vechi
                    for (int idL : E.legaturiSelectate) S.legaturi[idL]->selectat = false;
                    E.legaturiSelectate.clear();
                    
                    S.legaturi[id_legatura_sub_mouse]->selectat = true;
                    E.legaturiSelectate.push_back(id_legatura_sub_mouse);
                }
            }
        }
        else {
            if(!hasShift) {
                for (size_t i = 0 ; i < E.corpuriSelectate.size(); i++) {
                    S.corpuri[E.corpuriSelectate[i]].collider.selectat = 0;
                }
                E.corpuriSelectate.clear();

                for (int i : E.legaturiSelectate) S.legaturi[i]->selectat = false;
                E.legaturiSelectate.clear();
            }
            
            // Initializare Box Selection daca dam click in gol
            isBoxSelecting = true;
            boxSelectStartFizic = vec2(mouseX, mouseY);
            selectieInitiala = E.corpuriSelectate;
            selectieInitialaLegaturi = E.legaturiSelectate;
        }
    }
    
    static bool seMutaCorpuri = false;
    static std::vector<vec2> offsetArcLocal; // Retine punctul exact pe care l-ai apucat pe corp
    static vec2 pozitieMouseTrecut(mouseX, mouseY);
    vec2 mouseCurent(mouseX, mouseY);

    if(stangaClick && id_sub_mouse != -1) {
        if(std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), id_sub_mouse) != E.corpuriSelectate.end()) {
            seMutaCorpuri = true;
            
            // Salvam coordonatele locale ca sa tragem fix de coltul apucat, nu de centrul corpului
            offsetArcLocal.clear();
            for(int id : E.corpuriSelectate) {
                offsetArcLocal.push_back(S.corpuri[id].globalToLocal(mouseCurent));
            }
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (seMutaCorpuri) {
            if (E.mod_curent == MOD_EDITARE) {
                
                // --- MOD EDITARE: Mutare instanta (Teleportare) ---
                float deltaX = mouseCurent.x - pozitieMouseTrecut.x;
                float deltaY = mouseCurent.y - pozitieMouseTrecut.y;

                for (int id : E.corpuriSelectate) {
                    S.corpuri[id].pozitie.x += deltaX;
                    S.corpuri[id].pozitie.y += deltaY;

                    // Oprim inertiile sa nu fuga corpul cand dam drumul la click
                    S.corpuri[id].viteza.x = 0.0f;
                    S.corpuri[id].viteza.y = 0.0f;
                    S.corpuri[id].omega = 0.0f;
                }

                if (E.corpuriSelectate.size() > 0 && (deltaX != 0 || deltaY != 0)) {
                    for(size_t i = 0; i < S.corpuri.size(); i++){
                        S.corpuri[i].forte_desen.reseteaza();
                    }
                    S.seteazaForteExterne();
                    S.incarcaStare();
                }
                
            } else if (E.mod_curent == MOD_RULARE) {
                
                // --- MOD RULARE: Mouse Drag (Urmarire Kinematica) ---
                for (size_t i = 0; i < E.corpuriSelectate.size(); i++) {
                    int id = E.corpuriSelectate[i];
                    if (id == 0 || S.corpuri[id].M > 1e10f) continue; // Nu tragem de Lume

                    vec2 punctGlobal = (offsetArcLocal.size() > i) ? S.corpuri[id].localToGlobal(offsetArcLocal[i]) : S.corpuri[id].pozitie;
                    vec2 diferenta = mouseCurent - punctGlobal;
                    
                    // Cu cat e mai mare agilitatea, cu atat zboara mai repede dupa mouse
                    float agilitate = 15.0f; 
                    
                    // Suprascriem viteza. Corpul se va misca natural spre cursor fara sa fie oprit de frecari interne
                    S.corpuri[id].viteza.x = diferenta.x * agilitate;
                    S.corpuri[id].viteza.y = diferenta.y * agilitate;

                    // Rotatia: Il facem sa se rasuceasca natural daca tragi de un colt
                    vec2 r = punctGlobal - S.corpuri[id].pozitie; 
                    float cuplu = (r.x * diferenta.y - r.y * diferenta.x);
                    S.corpuri[id].omega = cuplu * agilitate * 0.5f;

                    // --- Desenam linia portocalie intre mouse si corp ---
                    if (E.elementeUI.size() < 4) E.elementeUI.resize(4);
                    E.elementeUI[3].activa = true;
                    E.elementeUI[3].tip = 4; 
                    E.elementeUI[3].x = (punctGlobal.x + mouseCurent.x) / 2.0f;
                    E.elementeUI[3].y = (punctGlobal.y + mouseCurent.y) / 2.0f;
                    E.elementeUI[3].phi = std::atan2(diferenta.y, diferenta.x);
                    E.elementeUI[3].dim1 = diferenta.modul();
                    E.elementeUI[3].dim2 = diferenta.modul();
                    E.elementeUI[3].col = {1.0f, 0.6f, 0.0f, 0.8f};
                }
                
                if (E.corpuriSelectate.size() > 0) {
                    S.incarcaStare();
                }
            }
        }
    } else {
        seMutaCorpuri = false; 
        if (E.elementeUI.size() >= 4) {
            E.elementeUI[3].activa = false; // Ascundem firul cand luam degetul de pe click
        }
    }
    pozitieMouseTrecut = mouseCurent;

    // Logica si randarea pentru Drag-Selection (Box Select)
    if (isBoxSelecting && stangaApasat) {
        float minX = std::min(boxSelectStartFizic.x, mouseCurent.x);
        float maxX = std::max(boxSelectStartFizic.x, mouseCurent.x);
        float minY = std::min(boxSelectStartFizic.y, mouseCurent.y);
        float maxY = std::max(boxSelectStartFizic.y, mouseCurent.y);

        // Curățăm selecția corpurilor din cadrul precedent al drag-ului și restaurăm starea inițială
        for (int id : E.corpuriSelectate) { S.corpuri[id].collider.selectat = 0; }
        E.corpuriSelectate = selectieInitiala;
        for (int id : E.corpuriSelectate) { S.corpuri[id].collider.selectat = 1; }

        // La fel si pentru legaturi
        for (int id : E.legaturiSelectate) { S.legaturi[id]->selectat = false; }
        E.legaturiSelectate = selectieInitialaLegaturi;
        for (int id : E.legaturiSelectate) { S.legaturi[id]->selectat = true; }

        // Selectăm corpurile din interiorul dreptunghiului
        for (size_t i = 1; i < S.corpuri.size(); i++) { // Ignorăm "Lumea" (index 0 / corpuri statice)
            if (!S.corpuri[i].activ || S.corpuri[i].M > 1e10f) continue;
            
            S.corpuri[i].seteazaBoundingBox(); // Ne asigurăm că AABB-ul e actualizat
            
            float minX_corp = S.corpuri[i].pozitie.x - S.corpuri[i].collider.bb.razaLatime;
            float maxX_corp = S.corpuri[i].pozitie.x + S.corpuri[i].collider.bb.razaLatime;
            float minY_corp = S.corpuri[i].pozitie.y - S.corpuri[i].collider.bb.razaInaltime;
            float maxY_corp = S.corpuri[i].pozitie.y + S.corpuri[i].collider.bb.razaInaltime;

            // Verificare intersecție AABB (Cutie Selecție vs Bounding Box Corp)
            if (maxX >= minX_corp && minX <= maxX_corp && maxY >= minY_corp && minY <= maxY_corp) {
                bool eraInitialSelectat = (std::find(selectieInitiala.begin(), selectieInitiala.end(), i) != selectieInitiala.end());
                
                if (hasShift && eraInitialSelectat) {
                    auto it = std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), i);
                    if (it != E.corpuriSelectate.end()) {
                        E.corpuriSelectate.erase(it);
                        S.corpuri[i].collider.selectat = 0;
                    }
                } else {
                    if (std::find(E.corpuriSelectate.begin(), E.corpuriSelectate.end(), i) == E.corpuriSelectate.end()) {
                        E.corpuriSelectate.push_back(i);
                        S.corpuri[i].collider.selectat = 1;
                    }
                }
            }
        }

        // Selectam legaturile din interiorul dreptunghiului
        for (size_t i = 0; i < S.legaturi.size(); i++) {
            if (!S.legaturi[i]->activ) continue;

            vec2 poz = S.legaturi[i]->getPozitie(S.corpuri);
            if (poz.x >= minX && poz.x <= maxX && poz.y >= minY && poz.y <= maxY) {
                bool eraInitialSelectat = (std::find(selectieInitialaLegaturi.begin(), selectieInitialaLegaturi.end(), i) != selectieInitialaLegaturi.end());
                
                if (hasShift && eraInitialSelectat) {
                    auto it = std::find(E.legaturiSelectate.begin(), E.legaturiSelectate.end(), i);
                    if (it != E.legaturiSelectate.end()) {
                        E.legaturiSelectate.erase(it);
                        S.legaturi[i]->selectat = false;
                    }
                } else {
                    if (std::find(E.legaturiSelectate.begin(), E.legaturiSelectate.end(), i) == E.legaturiSelectate.end()) {
                        E.legaturiSelectate.push_back(i);
                        S.legaturi[i]->selectat = true;
                    }
                }
            }
        }

        // Desenăm dreptunghiul vizual direct in coordonate OpenGL folosind o entitate UI
        if (E.elementeUI.size() < 3) E.elementeUI.resize(3);
        E.elementeUI[2].activa = true;
        E.elementeUI[2].tip = 2; // Dreptunghi
        E.elementeUI[2].x = (minX + maxX) / 2.0f;
        E.elementeUI[2].y = (minY + maxY) / 2.0f;
        E.elementeUI[2].dim1 = std::max(0.01f, maxX - minX); // evităm dimensiune 0
        E.elementeUI[2].dim2 = std::max(0.01f, maxY - minY);
        E.elementeUI[2].phi = 0.0f;
        E.elementeUI[2].col = {0.3f, 0.5f, 1.0f, 0.25f}; // Albastru semi-transparent
        
    } else if (!stangaApasat) {
        isBoxSelecting = false;
        if (E.elementeUI.size() >= 3) {
            E.elementeUI[2].activa = false;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // Deselectam corpuri
        for (size_t i = 0 ; i < E.corpuriSelectate.size(); i++) {
            S.corpuri[E.corpuriSelectate[i]].collider.selectat = 0;
        }
        E.corpuriSelectate.clear();

        // Deselectam legaturi
        for (int idL : E.legaturiSelectate) {
            S.legaturi[idL]->selectat = false;
        }
        E.legaturiSelectate.clear();
    }

    // --- Schimbarea formei cursorului ---
    if (!io.WantCaptureMouse) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);  // Mutarea camerei (Pan)
        } else if (isBoxSelecting && stangaApasat) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None); // Ascundem cursorul implicit al OS-ului
            
            // Desenam un cursor custom de tip "Crosshair" (tinta)
            ImDrawList* draw_list = ImGui::GetForegroundDrawList();
            ImVec2 center = io.MousePos;
            float line_len = 8.0f;
            float thickness = 1.5f;
            
            // Umbra subtila pentru a fi vizibil pe orice fundal luminos
            draw_list->AddLine(ImVec2(center.x - line_len, center.y), ImVec2(center.x + line_len, center.y), IM_COL32(0, 0, 0, 180), thickness + 1.5f);
            draw_list->AddLine(ImVec2(center.x, center.y - line_len), ImVec2(center.x, center.y + line_len), IM_COL32(0, 0, 0, 180), thickness + 1.5f);
            // Liniile albe ale tintei
            draw_list->AddLine(ImVec2(center.x - line_len, center.y), ImVec2(center.x + line_len, center.y), IM_COL32(255, 255, 255, 255), thickness);
            draw_list->AddLine(ImVec2(center.x, center.y - line_len), ImVec2(center.x, center.y + line_len), IM_COL32(255, 255, 255, 255), thickness);
        } else if (seMutaCorpuri && stangaApasat) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);  // Mutarea obiectelor selectate
        } else if (id_sub_mouse != -1 || id_legatura_sub_mouse != -1) { // Adaugat si pentru legaturi
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);       // Hover peste un obiect sau legatura
        } else {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);      // Cursorul implicit
        }
    }
}*/

void handleCameraControls(GLFWwindow *window, editor &E){
    
}

void processInput(GLFWwindow *window, sistem &S, editor &E){
    
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    E.updateMousePosition(window, zoomScale, aspect_ratio, cameraX, cameraY);
    handleCameraControls(window,E);
}

