#include "sistem.h"
#include "grafica.h"
#include "fizica.h" // Includem header-ul, nu fisierul sursa

// Constanta pentru conversia grade -> radiani
const float DEG_TO_RAD = 3.1415926535f / 180.0f;
float INF = 1e12f; 
int main(){
    std::cout << "==> Initializare Sistem..." << std::endl;
    
    
sistem S; 
S.setareConstantaGravitationala(9.81f); // Activam gravitatia
S.setareConstantaFrecareAer(0.05f);     // O frecare usoara cu aerul ajuta la stabilitate

// 2. Definire Corpuri
// Tavanul: Corp fix situat in originea axelor (sau unde doresti)
rigid tavan = rigid::Fix(0.0f, 5.0f); 
S.adaugaCorpuri(tavan);

// Masa: O bara sau o cutie de 2kg, situata initial mai jos de tavan
rigid masa = rigid::Bara(0.0f, 2.0f, 1.0f, 1.0f, 2.0f); 
S.adaugaCorpuri(masa);

// 3. Crearea Arcului (Oscilatorul Armonic)
// Parametri: CorpA, CorpB, X_globalA, Y_globalA, X_globalB, Y_globalB, k, d, l0
// Folosim k=100 pentru o elasticitate medie si d=5 pentru amortizare
// Daca lasi l0 implicit (-1.0f), el se va calcula ca distanta initiala (3.0 metri)
arc oscilator = arc::Creaza(S.corpuri[0], S.corpuri[1], 
                            0.0f, 5.0f,   // Prindere pe tavan
                            0.5f, 2.5f,   // Prindere coltul corpului
                            100.0f, 5.0f); 
S.adaugaArcuri(oscilator);

// 4. Pregatire finala
S.incarcaStare();
S.seteazaMatriceInertie();


    // Initializare OpenGL
    std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
    unsigned int shaderProgram;
    GLFWwindow* window = openGLWindow(shaderProgram);

    if(window != NULL){
        // Initializare Buffere (VAO, VBO)
        unsigned int VAO, VBO;
        initBuffers(VAO, VBO);

        // Buffer local pentru coordonate
        // 6 valori (x, y, phi, w, h, type) * (nr_corpuri + nr_legaturi)
        std::vector<float> vertexBuffer(6 * (S.nr_corpuri + S.nr_legaturi + S.arcuri.size()));

        // Variabile de timp
        float t = 0.0f, dt = 0.001f;
        bool running_flag = 1;
        bool arata_energie_flag = 1;

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window, dt, running_flag);

            // 2. Fizica 
            // std::cout << "Pas fizica..." << std::endl; 
            if(running_flag){
                for(int i = 0; i < 20; i++) {   //facem calculele de mai multe ori intre cadre, pentru ca nu avem nevoie de mai mult de 60 de cadre pe secunda
                    S.stare = RK4(S, dt, t);
                    S.seteazaStare();       //muta datele din matrice, in obiecte
                    verificarCiocniri(S);   //lucreaza pe variabilele din obiecte
                    S.incarcaStare();       //muta datele din obiecte in matricea de stare

                    t += dt;
                }
            }
           
            // 3. Randare
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            drawSystem(S, VAO, VBO, shaderProgram, vertexBuffer.data());

            glfwSwapBuffers(window);
            glfwPollEvents();

            float energie;
            if(running_flag) frameCount++;
            if(frameCount % 100 == 0){
                 std::cout << "Cadre randate: " << frameCount << " Timp simulat: " << t <<" | dt current: " << dt <<std::endl;
                 if(arata_energie_flag){
                    energie = calculeazaEnergiaTotala(S, S.g);
                    std::cout << "Energie: " << energie / 1000.0f <<" KJ " <<std::endl;
                 }
            }
        }
        
        glfwTerminate();
    }
    return 0;
}