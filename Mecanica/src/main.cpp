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
        // 17 valori (x, y, phi, w, h, type, red, green, blue, alpha, tip, viteza_x, viteza_x, omega, acceleratie_x, acceleratie_y, epsilon) * (nr_corpuri + nr_legaturi)
        std::vector<float> vertexBuffer(17 * (S.corpuri.size() + S.legaturi.size() + S.arcuri.size()));

        // Variabile de timp
        float t = 0.0f, dt = 0.001f;

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window, dt, S, E);
            // 2. Pre-Generare GUI (Colecteaza evenimentele OS)
            startFrameGUI();

            // 3. Fizica 
            if(E.mod_curent == MOD_RULARE){

                for(int i = 0; i < S.corpuri.size(); i++){
                    S.corpuri[i].forte_desen.reseteaza();
                    S.corpuri[i].forte_desen.viteza_cadru_trecut = S.corpuri[i].viteza;
                    S.corpuri[i].forte_desen.omega_cadru_trecut = S.corpuri[i].omega;
                }

                for(int i = 0; i < 20; i++) {   //facem calculele de mai multe ori intre cadre, pentru ca nu avem nevoie de mai mult de 60 de cadre pe secunda
                    S.stare = RK4(S, dt, t);
                    S.seteazaStare();       //muta datele din matrice, in obiecte
                    S.plafonareViteze();
                    verificarCiocniri(S,E);   //lucreaza pe variabilele din obiecte
                    S.incarcaStare();       //muta datele din obiecte in matricea de stare

                    t += dt;
                }
                adaugaForteContinueVizuale(S);
                salveazaDateCinematiceVizuale(S,dt,20);
            } else {
                S.incarcaStare();
                verificarCiocniri(S,E);
            }
           
            // 4. Randare OpenGL Lume
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Calculam dinamic memoria necesara pentru buffer-ul video
            size_t nr_forte = 0;
            if (E.flag.arata_forte) {
                for (const auto& corp : S.corpuri) {
                    if (corp.activ) {
                        nr_forte += corp.forte_desen.forte.size();
                    }
                }
            }
            size_t total_elemente = S.corpuri.size() + S.legaturi.size() + S.arcuri.size() + E.elementeUI.size() + nr_forte;
            vertexBuffer.resize(17 * total_elemente);

            drawSystem(S,E, VAO, VBO, shaderProgram, vertexBuffer.data());

            // 5. Randare ImGui (peste scena de fizica)
            float energie = 0.0f;
            if(E.flag.arata_energie){
                energie = calculeazaEnergiaTotala(S, S.g);
            }
            renderPanouDeControl(S,E,dt, t, energie);
            if(E.corpApasat != -1)
               renderInspector(S,E);
            endFrameGUI();

            glfwSwapBuffers(window);
            glfwPollEvents();

            if(E.mod_curent == MOD_RULARE) frameCount++;
             // [DEBUG] Afisam starea sistemului regulat (aproximativ 1 data pe secunda)
                if (frameCount % 60 == 0) { 
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
        }
        
        // Curatam GUI inainte sa inchidem programul
        cleanupGUI();
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glfwTerminate();
    }
    return 0;
}