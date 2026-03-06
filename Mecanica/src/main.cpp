#include "sistem.h"
#include "grafica.h"
#include "fizica.h" // Includem header-ul, nu fisierul sursa

// Constanta pentru conversia grade -> radiani
const float DEG_TO_RAD = 3.1415926535f / 180.0f;
float INF = 1e12f; 
int main(){
    std::cout << "==> Initializare Sistem..." << std::endl;
    
    /*sistem S;            // pendul triplu

    S.setareConstante(20.0f, 5.0f); 
    S.setareConstantaGravitationala(9.81f);

    // rigid::Bara(index, x, y, Lungime, Grosime, Masa)
    rigid LUME = rigid::Fix(0, 0);
    rigid bara1 = rigid::Bara(2.0f, 0.0f, 4.0f, 0.4f, 4.0f);
    rigid bara2 = rigid::Bara(6.0f, 0.0f, 4.0f, 0.4f, 4.0f); 
    rigid bara3 = rigid::Bara(10.0f, 0.0f, 4.0f, 0.4f, 4.0f);

    // Definim legaturile folosind coordonate GLOBALE

    // Ex: Articulatie intre LUME(0) si bara1(1) la punctul (0,0)
    legatura* fixare = articulatie::Creaza(LUME, bara1, 0.0f, 0.0f);
    // Ex: Articulatie intre bara1(1) si bara2(2) la punctul (4,0) - unde se termina prima si incepe a doua
    legatura* art1   = articulatie::Creaza(bara1, bara2, 4.0f, 0.0f);
    legatura* art2   = articulatie::Creaza(bara2, bara3, 8.0f, 0.0f);

    S.adaugaCorpuri(LUME);
    S.adaugaCorpuri(bara1);
    S.adaugaCorpuri(bara2);
    S.adaugaCorpuri(bara3);
    S.adaugaLegaturi(fixare); 
    S.adaugaLegaturi(art1);
    S.adaugaLegaturi(art2);
    */

    /*
    sistem S;            //pod 

    S.setareConstante(20.0f, 5.0f); 
    S.setareConstantaGravitationala(9.81f);

    rigid LUME = rigid::Fix(0,0,0);

    rigid veriga1 = rigid::Bara(-7.5f, 0.0f, 1.0f, 0.2f, 2.0f);
    rigid veriga2 = rigid::Bara(-6.5f, 0.0f, 1.0f, 0.2f, 2.0f);
    rigid veriga3 = rigid::Bara(-5.5f, 0.0f, 1.0f, 0.2f, 2.0f);
    rigid veriga4 = rigid::Bara(-4.5f, 0.0f, 1.0f, 0.2f, 2.0f);
    rigid veriga5 = rigid::Bara(-3.5f, 0.0f, 1.0f, 0.2f, 2.0f);
    rigid veriga6 = rigid::Bara(-2.5f, 0.0f, 1.0f, 0.2f, 2.0f);

    legatura* fixare1 = articulatie::Creaza(LUME,    veriga1, -8.0f, 0.0f);
    legatura* art12   = articulatie::Creaza(veriga1, veriga2, -7.0f, 0.0f);
    legatura* art23   = articulatie::Creaza(veriga2, veriga3, -6.0f, 0.0f);
    legatura* art34   = articulatie::Creaza(veriga3, veriga4, -5.0f, 0.0f);
    legatura* art45   = articulatie::Creaza(veriga4, veriga5, -4.0f, 0.0f);
    legatura* art56   = articulatie::Creaza(veriga5, veriga6, -3.0f, 0.0f);
    legatura* fixare2 = articulatie::Creaza(veriga6, LUME,    -2.0f, 0.0f);

    S.adaugaCorpuri(LUME);
    S.adaugaCorpuri(veriga1);
    S.adaugaCorpuri(veriga2);
    S.adaugaCorpuri(veriga3);
    S.adaugaCorpuri(veriga4);
    S.adaugaCorpuri(veriga5);
    S.adaugaCorpuri(veriga6);

    S.adaugaLegaturi(fixare1); 
    S.adaugaLegaturi(art12);
    S.adaugaLegaturi(art23);
    S.adaugaLegaturi(art34);
    S.adaugaLegaturi(art45);
    S.adaugaLegaturi(art56);
    S.adaugaLegaturi(fixare2);
    */

    /*
    sistem S;        //lant

    S.setareConstante(20.0f, 5.0f); 
    S.setareConstantaGravitationala(9.81f);

    rigid LUME = rigid::Fix(0,0,0);

    rigid veriga1 = rigid::Bara(-7.5f, 0.0f, 1.0f, 0.2f, 30.0f);
    rigid veriga2 = rigid::Bara(-6.5f, 0.0f, 1.0f, 0.2f, 30.0f);
    rigid veriga3 = rigid::Bara(-5.5f, 0.0f, 1.0f, 0.2f, 30.0f);
    rigid veriga4 = rigid::Bara(-4.5f, 0.0f, 1.0f, 0.2f, 30.0f);
    rigid veriga5 = rigid::Bara(-3.5f, 0.0f, 1.0f, 0.2f, 30.0f);
    rigid veriga6 = rigid::Bara(-2.5f, 0.0f, 1.0f, 0.2f, 30.0f);

    
    legatura* art12   = articulatie::Creaza(veriga1, veriga2, -7.0f, 0.0f);
    legatura* art23   = articulatie::Creaza(veriga2, veriga3, -6.0f, 0.0f);
    legatura* art34   = articulatie::Creaza(veriga3, veriga4, -5.0f, 0.0f);
    legatura* art45   = articulatie::Creaza(veriga4, veriga5, -4.0f, 0.0f);
    legatura* art56   = articulatie::Creaza(veriga5, veriga6, -3.0f, 0.0f);
    legatura* fixare = articulatie::Creaza(LUME,    veriga6, -2.0f, 0.0f);

    S.adaugaCorpuri(LUME);
    S.adaugaCorpuri(veriga1);
    S.adaugaCorpuri(veriga2);
    S.adaugaCorpuri(veriga3);
    S.adaugaCorpuri(veriga4);
    S.adaugaCorpuri(veriga5);
    S.adaugaCorpuri(veriga6);

    S.adaugaLegaturi(fixare); 
    S.adaugaLegaturi(art12);
    S.adaugaLegaturi(art23);
    S.adaugaLegaturi(art34);
    S.adaugaLegaturi(art45);
    S.adaugaLegaturi(art56);
    */

    
    sistem S; 

    S.setareConstante(20.0f, 5.0f); 
    S.setareConstantaGravitationala(9.81f);

    rigid LUME = rigid::Fix(0, 0);
    rigid bara1 = rigid::Bara(2.0f, 0.0f, 4.0f, 0.4f, 2.0f);
    bara1.phi = 0.0f;
    rigid bara2 = rigid::Bara(4.0f, -2.0f, 4.0f, 0.4f, 2.0f);
    bara2.phi = -90.0f * DEG_TO_RAD; 

    legatura* tavan = articulatie::Creaza(LUME, bara1, 0.0f, 0.0f);
    legatura* sudura = incastrare::Creaza(bara1, bara2, 4.0f, 0.0f);

    S.adaugaCorpuri(LUME);
    S.adaugaCorpuri(bara1);
    S.adaugaCorpuri(bara2);

    S.adaugaLegaturi(tavan);
    S.adaugaLegaturi(sudura);

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
        float* vertexBuffer = new float[6 * (S.nr_corpuri + S.nr_legaturi)];

        // Variabile de timp
        float t = 0.0f;
        float dt = 0.001f; // Pasul de timp redus pentru stabilitate

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window);

            // 2. Fizica (RK4)
            // std::cout << "Pas fizica..." << std::endl; // Decomenteaza daca crapa aici
            for(int i = 0; i < 10; i++) {
                S.stare = RK4(S, dt, t);
                S.seteazaStare(); // Actualizam obiectele rigid din matricea stare
                t += dt;
            }

            // 3. Randare
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            std::vector<float> vertexBuffer(6 * (S.nr_corpuri + S.nr_legaturi));
            drawSystem(S, VAO, VBO, shaderProgram, vertexBuffer.data());

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