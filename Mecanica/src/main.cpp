#include "sistem.h"
#include "grafica.h"
#include "fizica.h" // Includem header-ul, nu fisierul sursa

// Constanta pentru conversia grade -> radiani
const float DEG_TO_RAD = 3.1415926535f / 180.0f;
float INF = 1e12f; 
int main(){
    std::cout << "==> Initializare Sistem..." << std::endl;
    
    
    sistem S; 
    S.setareConstante(20.0f, 5.0f); 
    S.setareConstantaGravitationala(9.81f);
    S.setareConstantaFrecareAer(0.1f);

    // 1. Podeaua
    rigid podea = rigid::Bara(0.0f, -8.0f, 20.0f, 2.0f, 1e12f);
    S.adaugaCorpuri(podea);

    // 2. Pivotul invizibil al balansoarului (punct fix pe ecran)
    rigid pivot = rigid::Fix(0.0f, -4.0f);
    S.adaugaCorpuri(pivot);

    // 3. Scândura balansoarului (10 metri lungime, masă 10kg)
    rigid scandura = rigid::Bara(0.0f, -4.0f, 10.0f, 0.4f, 10.0f);
    S.adaugaCorpuri(scandura);

    // Prindem scândura de pivot fix pe mijlocul ei
    legatura* art = articulatie::Creaza(pivot, scandura, 0.0f, -4.0f);
    S.adaugaLegaturi(art);

    // 4. "Muniția" (O cutie ușoară care stă pe capătul din dreapta)
    rigid munitie = rigid::Bara(4.0f, -3.0f, 1.0f, 1.0f, 2.0f);
    S.adaugaCorpuri(munitie);

    // 5. Greutatea care cade de sus (Masa mare, 50kg, cade pe capătul din stânga)
    rigid greutate = rigid::Bara(-4.0f, 6.0f, 2.0f, 2.0f, 50.0f);
    S.adaugaCorpuri(greutate);

    // 6. Câteva cutii suspendate pe care muniția ar trebui să le lovească în zbor
    for(int i = 0; i < 3; i++) {
        rigid obstacol = rigid::Bara(4.0f + i * 0.5f, 3.0f + i * 2.0f, 1.2f, 1.2f, 1.0f);
        S.adaugaCorpuri(obstacol);
    }

    // Setăm coeficienții înainte de a inițializa matricele
    S.seteazaCoeficientFrecare(0.3f); // Frecare mai mare ca muniția să nu alunece de pe scândură prea repede
    S.seteazaCoeficientRestituire(0.4f); // Scădem elasticitatea ca să nu sară greutatea aiurea

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
        float t = 0.0f;
        float dt = 0.001f; // Pasul de timp redus pentru stabilitate

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window);

            // 2. Fizica 
            // std::cout << "Pas fizica..." << std::endl; 
            for(int i = 0; i < 20; i++) {   //facem calculele de mai multe ori intre cadre, pentru ca nu avem nevoie de mai mult de 60 de cadre pe secunda
                S.stare = RK4(S, dt, t);
                 S.seteazaStare();       //muta datele din matrice, in obiecte
                verificarCiocniri(S);   //lucreaza pe variabilele din obiecte
                S.incarcaStare();       //muta datele din obiecte in matricea de stare

                t += dt;
            }
           
            // 3. Randare
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            drawSystem(S, VAO, VBO, shaderProgram, vertexBuffer.data());

            glfwSwapBuffers(window);
            glfwPollEvents();

            float energie;
            
            frameCount++;
            if(frameCount % 100 == 0){
                 std::cout << "Cadre randate: " << frameCount << " Timp simulat: " << t << std::endl;
                 float energie = calculeazaEnergiaTotala(S, S.g);
                std::cout << "Energie: " << energie / 1000.0f <<" KJ " <<std::endl;
            }
        }
        
        glfwTerminate();
    }
    return 0;
}