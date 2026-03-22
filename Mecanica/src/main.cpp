#include "sistem.h"
#include "grafica.h"
#include "fizica.h"
#include <cmath>
#include <iostream>

const float PI = 3.1415926535f;

int main() {
    std::cout << "==> Initializare Sistem Static si Rostogolire..." << std::endl;
    
    sistem S; 
    S.setareConstantaGravitationala(9.81f);

    // ============================================================
    // PARAMETRII PROBLEMEI
    // ============================================================
    const float alpha = 33.0f * PI / 180.0f;
    
    const float L_OAB = 10.0f;  // Lungimea barei articulate
    const float M_OAB = 15.0f;  // Greutatea Q

    const float L_O1B = 4.0f;   // Lungimea barei incastrate
    const float M_O1B = 20.0f;  // Greutatea P (o facem masiva ca sa fie si mai stabila)

    const float R_disc = 0.5f;  // Raza discului C
    const float M_disc = 3.0f;  // Masa discului

    // Grosimile barelor (ne trebuie pentru a nu le genera una in alta)
    const float grosime_OAB = 0.4f;
    const float grosime_O1B = 0.4f;

    // ============================================================
    // 1. Bara inclinata OAB (Articulata in O)
    // ============================================================
    float cx_OAB = (L_OAB / 2.0f) * std::cos(alpha);
    float cy_OAB = (L_OAB / 2.0f) * std::sin(alpha);

    rigid bara_OAB = rigid::Bara(cx_OAB, cy_OAB, L_OAB, grosime_OAB, M_OAB);
    bara_OAB.phi = alpha;
    bara_OAB.collider.culoare = {0.8f, 0.6f, 0.2f, 1.0f};
    bara_OAB.material = materiale::Lemn;
    S.adaugaCorpuri(bara_OAB);
    int id_OAB = S.corpuri.size() - 1;

    // ============================================================
    // 2. Bara orizontala O1B (Incastrata in dreapta, la O1)
    // ============================================================
    // Alegem punctul B la 6 metri distanta de O, pe bara OAB
    float dist_B = 6.0f; 
    float B_x = dist_B * std::cos(alpha);
    float B_y = dist_B * std::sin(alpha);

    // Calculam cat trebuie coborata bara orizontala ca sa atinga exact dedesubtul barei inclinate
    float offset_y_OAB = (grosime_OAB / 2.0f) / std::cos(alpha);
    float offset_y_O1B = grosime_O1B / 2.0f;
    float coborare_totala = offset_y_OAB + offset_y_O1B + 0.05f; // + 0.05f luft de siguranta

    float cx_O1B = B_x + (L_O1B / 2.0f);
    float cy_O1B = B_y - coborare_totala;

    rigid bara_O1B = rigid::Bara(cx_O1B, cy_O1B, L_O1B, grosime_O1B, M_O1B);
    bara_O1B.phi = 0.0f;
    bara_O1B.collider.culoare = {0.4f, 0.7f, 1.0f, 1.0f};
    bara_O1B.material = materiale::Lemn;
    S.adaugaCorpuri(bara_O1B);
    int id_O1B = S.corpuri.size() - 1;

    // ============================================================
    // 3. Discul C (Sus pe bara, gata sa alunece/rostogoleasca)
    // ============================================================
    float dist_C = 8.5f; // Il punem aproape de capatul de sus
    
    // Coordonatele punctului de pe centrul barei
    float axa_x = dist_C * std::cos(alpha);
    float axa_y = dist_C * std::sin(alpha);

    // Ne ridicam pe normala la bara
    float normal_x = -std::sin(alpha);
    float normal_y =  std::cos(alpha);
    
    float distanta_ridicare = R_disc + (grosime_OAB / 2.0f) + 0.05f; // luft

    rigid disc = rigid::Disc(axa_x + normal_x * distanta_ridicare, 
                             axa_y + normal_y * distanta_ridicare, 
                             R_disc, M_disc);
    disc.collider.culoare = {1.0f, 0.3f, 0.3f, 1.0f};
    
    // Il facem din cauciuc ca sa prinda aderenta si sa se rostogoleasca frumos
    disc.material = materiale::Cauciuc; 
    S.adaugaCorpuri(disc);

    // ============================================================
    // 4. Legaturi (Articulatie in O si Incastrare in O1)
    // ============================================================
    // Calculam coordonatele punctului O1 (capatul drept al barei orizontale)
    float O1_x = cx_O1B + (L_O1B / 2.0f);
    float O1_y = cy_O1B;

    // Legam bara OAB direct de Lume (S.corpuri[0]) in punctul O (0,0)
    S.adaugaLegaturi(articulatie::Creaza(S.corpuri[0], S.corpuri[id_OAB], 0.0f, 0.0f));

    // Legam bara O1B direct de Lume (S.corpuri[0]) in punctul O1
    S.adaugaLegaturi(incastrare::Creaza(S.corpuri[0], S.corpuri[id_O1B], O1_x, O1_y));

    // ============================================================
    // 5. PREGATIREA MATRICELOR (NU STERGE ACESTE LINII!)
    // ============================================================
    S.setareConstanteStabilizare(10000.0f, 500.0f);
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

        // Initializare ImGui (trebuie facuta dupa openGLWindow)
        setupGUI(window);

        // Buffer local pentru coordonate
        // 10 valori (x, y, phi, w, h, type, red, green, blue, alpha) * (nr_corpuri + nr_legaturi)
        std::vector<float> vertexBuffer(10 * (S.corpuri.size() + S.legaturi.size() + S.arcuri.size()));

        // Variabile de timp
        float t = 0.0f, dt = 0.001f;
        bool running_flag = 0;
        bool arata_energie_flag = 1;

        // Bucla principala
        std::cout << "==> Intrare in bucla de randare..." << std::endl;
        int frameCount = 0;
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window, dt, running_flag,S);

            // 2. Pre-Generare GUI (Colecteaza evenimentele OS)
            startFrameGUI();

            // 3. Fizica 
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
           
            // 4. Randare OpenGL Lume
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Asiguram memorie GPU pentru corpurile care tocmai s-au adaugat din meniu
            vertexBuffer.resize(10 * (S.corpuri.size() + S.legaturi.size() + S.arcuri.size()));

            drawSystem(S, VAO, VBO, shaderProgram, vertexBuffer.data());

            // 5. Randare ImGui (peste scena de fizica)
            float energie = 0.0f;
            if(arata_energie_flag){
                energie = calculeazaEnergiaTotala(S, S.g);
            }
            renderPanouDeControl(dt, running_flag, arata_energie_flag, t, energie);
            renderPanouDeAdaugatCorpuri(S);
            endFrameGUI();

            glfwSwapBuffers(window);
            glfwPollEvents();

            if(running_flag) frameCount++;
            if(frameCount % 100 == 0 && running_flag == 1){
                 std::cout << "Cadre randate: " << frameCount << " Timp simulat: " << t <<" | dt current: " << dt <<std::endl;
                 if(arata_energie_flag){
                    energie = calculeazaEnergiaTotala(S, S.g);
                    std::cout << "Energie: " << energie / 1000.0f <<" KJ " <<std::endl;
                 }
            }
        }
        
        // Curatam GUI inainte sa inchidem programul
        cleanupGUI();
        glfwTerminate();
    }
    return 0;
}