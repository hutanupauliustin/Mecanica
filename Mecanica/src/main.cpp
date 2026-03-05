#include "sistem.h"
#include "grafica.h"
#include "fizica.h" // Includem header-ul, nu fisierul sursa

// Constanta pentru conversia grade -> radiani
const float DEG_TO_RAD = 3.1415926535f / 180.0f;
float INF = 1e12f; // Folosim o valoare finita mare (10^12) in loc de infinit pentru a evita erori numerice (NaN)

int main(){
    std::cout << "==> Initializare Sistem..." << std::endl;
    sistem S(3,2); 

    // Crestem constantele de stabilizare (Baumgarte) pentru a corecta erorile numerice mai eficient
    S.setareConstante(20.0f, 5.0f); 
    S.setareConstantaGravitationala(9.81f);


    rigid LUME(0,0,0,INF,INF);
    
    // --- CONFIGURATIE STABILA (Dublu Pendul Orizontal) ---
    // Bara 1: Lungime 4, Centru la (2,0), Masa 4
    rigid bara1(2.0f, 0.0f, 0.0f, 4.0f, 4.0f);
    // Bara 2: Lungime 4, Centru la (6,0), Masa 4 (conectata in continuarea primei)
    rigid bara2(6.0f, 0.0f, 0.0f, 4.0f, 4.0f); 

    // Legam LUME(0,0) de capatul stang al barei 1 (-2.0 fata de centru)
    legatura* fixare =  new articulatie(0, 1, 0.0f, 0.0f, -2.0f, 0.0f);
    
    // Legam capatul drept al barei 1 (2.0) de capatul stang al barei 2 (-2.0)
    legatura* art = new articulatie(1, 2, 2.0f, 0.0f, -2.0f, 0.0f);

    S.adaugaCorpuri(LUME);
    S.adaugaCorpuri(bara1);
    S.adaugaCorpuri(bara2);
    S.adaugaLegaturi(fixare); 
    S.adaugaLegaturi(art);

    std::cout << "==> Incarcare Stare..." << std::endl;
    // Initializam starea sistemului si matricea de inertie
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
        // 2 coordonate (x,y) * (nr_corpuri + nr_legaturi)
        float* vertexBuffer = new float[2 * (S.nr_corpuri + S.nr_legaturi)];

        // Variabile de timp
        float t = 0.0f;
        float dt = 0.01f; // Pasul de timp pentru simulare

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window);

            // 2. Fizica (RK4)
            // std::cout << "Pas fizica..." << std::endl; // Decomenteaza daca crapa aici
            S.stare = RK4(S, dt, t);
            S.seteazaStare(); // Actualizam obiectele rigid din matricea stare
            t += dt;

            // 3. Randare
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            drawSystem(S, VAO, VBO, shaderProgram, vertexBuffer);

            glfwSwapBuffers(window);
            glfwPollEvents();
            
            frameCount++;
            if(frameCount % 100 == 0) std::cout << "Cadre randate: " << frameCount << " Timp simulat: " << t << std::endl;
        }
        
        delete[] vertexBuffer;
        glfwTerminate();
    }
    return 0;
}