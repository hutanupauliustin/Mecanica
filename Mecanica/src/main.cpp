#include "sistem.h"
#include "grafica.h"
#include "fizica.h"
#include "editor.h" 
#include "gui.h"
#include "scena.h"
#include "editor.h"
#include <cmath>
#include <iostream>

const float PI = 3.1415926535f;

int main() {
    std::cout << "==> Initializare Sistem Static si Rostogolire..." << std::endl;
    
    sistem S; 
    editor E;
    S.setareConstantaGravitationala(9.81f);

   incarcaScenaInitiala(S);

    // Initializare OpenGL
    std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
    unsigned int shaderProgram;
    GLFWwindow* window = openGLWindow(shaderProgram);

    if(window != NULL){
        // Initializare Buffere (VAO, VBO)
        unsigned int VAO, VBO;
        initBuffers(VAO, VBO);

        // Initializare ImGui (trebuie facuta dupa openGLWindow)
        setupGUI(window);

        // Buffer local pentru coordonate
        // 10 valori (x, y, phi, w, h, type, red, green, blue, alpha) * (nr_corpuri + nr_legaturi)
        std::vector<float> vertexBuffer(11 * (S.corpuri.size() + S.legaturi.size() + S.arcuri.size()));

        // Variabile de timp
        float t = 0.0f, dt = 0.001f;
        bool running_flag = 0;
        bool arata_energie_flag = 1;

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window, dt, running_flag, S, E);

            // 2. Pre-Generare GUI (Colecteaza evenimentele OS)
            startFrameGUI();

            running_flag = (E.mod_curent != MOD_EDITARE);

            // 3. Fizica 
            if(running_flag){
                for(int i = 0; i < 20; i++) {   //facem calculele de mai multe ori intre cadre, pentru ca nu avem nevoie de mai mult de 60 de cadre pe secunda
                    S.stare = RK4(S, dt, t);
                    S.seteazaStare();       //muta datele din matrice, in obiecte
                    S.plafonareViteze();
                    verificarCiocniri(S,E);   //lucreaza pe variabilele din obiecte
                    S.incarcaStare();       //muta datele din obiecte in matricea de stare

                    bool eroare_matematica = false;
                    for (int c = 0; c < S.corpuri.size(); c++) {
                        if (S.corpuri[c].activ && (std::isnan(S.corpuri[c].pozitie.x) || std::isnan(S.corpuri[c].pozitie.y))) {
                            eroare_matematica = true; break;
                        }
                    }
                    if (eroare_matematica) {
                        std::cout << "\n[EROARE FATALA] S-a detectat 'NaN' in fizica! (Forte infinite). Sistemul a fost salvat de la Crash!" << std::endl;
                        running_flag = false;
                        break; 
                    }

                    t += dt;
                }
            } else {
                S.incarcaStare();
                verificarCiocniri(S,E);
            }
           
            // 4. Randare OpenGL Lume
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Asiguram memorie GPU pentru corpurile care tocmai s-au adaugat din meniu
            vertexBuffer.resize(11 * (S.corpuri.size() + S.legaturi.size() + S.arcuri.size() + E.elementeUI.size()));

            drawSystem(S,E, VAO, VBO, shaderProgram, vertexBuffer.data());

            // 5. Randare ImGui (peste scena de fizica)
            float energie = 0.0f;
            if(arata_energie_flag){
                energie = calculeazaEnergiaTotala(S, S.g);
            }
            renderPanouDeControl(dt, running_flag, arata_energie_flag, t, energie);
            renderPanouDeAdaugatCorpuri(S,E);
            endFrameGUI();

            glfwSwapBuffers(window);
            glfwPollEvents();

            if(running_flag) frameCount++;
             // [DEBUG] Afisam starea sistemului regulat (aproximativ 1 data pe secunda)
                if (frameCount % 60 == 0) { 
                    std::cout << "\n[Fizica] Cadru " << frameCount << " | Timp: " << t << " | Corpuri in sistem: " << S.corpuri.size() << std::endl;
                    for (size_t c = 0; c < S.corpuri.size(); c++) {
                        if(S.corpuri[c].activ == 0) continue; // Sarim peste cele sterse
                        std::cout << "  -> Corp " << c 
                                  << " | Pos: (" << S.corpuri[c].pozitie.x << ", " << S.corpuri[c].pozitie.y << ")"
                                  << " | Viteza: (" << S.corpuri[c].viteza.x << ", " << S.corpuri[c].viteza.y << ")"
                                  << " | Masa: " << S.corpuri[c].M << std::endl;
                    }
                }
        }
        
        // Curatam GUI inainte sa inchidem programul
        cleanupGUI();
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glfwTerminate();
    }
    return 0;
}