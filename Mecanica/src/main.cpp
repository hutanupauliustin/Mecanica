#include "sistem.h"
#include "grafica_input.h"
#include "fizica.h" // Includem header-ul, nu fisierul sursa

// Constanta pentru conversia grade -> radiani
const float DEG_TO_RAD = 3.1415926535f / 180.0f;
float INF = 1e12f; 
int main(){
    std::cout << "==> Initializare Sistem..." << std::endl;
    
    // 1. Initializare Sistem
sistem S; 
S.setareConstantaGravitationala(9.81f);
S.setareConstantaFrecareAer(0.2f); 

// 2. Adaugam Podeaua
// Folosim o masa "infinita" (INF) ca sa se comporte ca un corp static.
// O punem mai jos (ex: y = -2.0f) si o facem suficient de lata sa prinda tot grid-ul.
rigid podea = rigid::Bara(0.0f, -2.0f, 15.0f, 1.0f, INF); 
S.adaugaCorpuri(podea);

// Parametrii retelei
const int randuri = 5;
const int coloane = 5;
float spatiere = 1.0f; 
float startX = -2.0f;  
float startY = 6.0f;   

// 3. Generarea corpurilor din grid (cu rotatie initiala)
float unghi_inclinare = 0.1f;
float cos_u = std::cos(unghi_inclinare);
float sin_u = std::sin(unghi_inclinare);

for(int r = 0; r < randuri; r++) {
    for(int c = 0; c < coloane; c++) {
        // Coordonatele locale in cadrul grilei (fata de coltul stanga-sus)
        float local_x = c * spatiere;
        float local_y = -r * spatiere; 
        
        // Aplicam matricea de rotatie 2D
        float rot_x = local_x * cos_u - local_y * sin_u;
        float rot_y = local_x * sin_u + local_y * cos_u;
        
        // Translatam grila rotita in pozitia de start pe ecran
        float x = startX + rot_x;
        float y = startY + rot_y;
        
        // Corpurile au masa normala (1kg) si sunt orientate cu acelasi unghi
        rigid corp = rigid::Bara(x, y, 0.4f, 0.4f, 1.0f); 
        corp.phi = unghi_inclinare; // Rotim vizual si cutiile ca sa se alinieze cu plasa
        
        S.adaugaCorpuri(corp);
    }
}

// 4. Generarea arcurilor
float k_retea = 250.0f; 
float d_retea = 15.0f;  

// --- A. Arcurile Orizontale si Verticale ---
for(int r = 0; r < randuri; r++) {
    for(int c = 0; c < coloane; c++) {
        
        int idxCurent = 3 + (r * coloane + c);
        
        // Conexiune Orizontala
        if(c < coloane - 1) {
            int idxDreapta = 3 + (r * coloane + (c + 1));
            arc aOriz = arc::Creaza(S.corpuri[idxCurent], S.corpuri[idxDreapta], 
                                    S.corpuri[idxCurent].x, S.corpuri[idxCurent].y,
                                    S.corpuri[idxDreapta].x, S.corpuri[idxDreapta].y,
                                    k_retea, d_retea, spatiere);
            S.adaugaArcuri(aOriz);
        }
        
        // Conexiune Verticala
        if(r < randuri - 1) {
            int idxJos = 3 + ((r + 1) * coloane + c);
            arc aVert = arc::Creaza(S.corpuri[idxCurent], S.corpuri[idxJos], 
                                    S.corpuri[idxCurent].x, S.corpuri[idxCurent].y,
                                    S.corpuri[idxJos].x, S.corpuri[idxJos].y,
                                    k_retea, d_retea, spatiere);
            S.adaugaArcuri(aVert);
        }
    }
}

// --- B. Arcurile Diagonale ---
float spatiere_diag = spatiere * 1.41421356f; 
float k_diag = k_retea * 0.8f; 
float d_diag = d_retea;

for(int r = 1; r < randuri - 1; r++) {
    for(int c = 1; c < coloane - 1; c++) {
        
        int stanga_sus  = 3 + (r * coloane + c);
        int dreapta_sus = 3 + (r * coloane + (c + 1));
        int stanga_jos  = 3 + ((r + 1) * coloane + c);
        int dreapta_jos = 3 + ((r + 1) * coloane + (c + 1));

        arc aDiag1 = arc::Creaza(S.corpuri[stanga_sus], S.corpuri[dreapta_jos], 
                                 S.corpuri[stanga_sus].x, S.corpuri[stanga_sus].y,
                                 S.corpuri[dreapta_jos].x, S.corpuri[dreapta_jos].y,
                                 k_diag, d_diag, spatiere_diag);
        S.adaugaArcuri(aDiag1);

        arc aDiag2 = arc::Creaza(S.corpuri[dreapta_sus], S.corpuri[stanga_jos], 
                                 S.corpuri[dreapta_sus].x, S.corpuri[dreapta_sus].y,
                                 S.corpuri[stanga_jos].x, S.corpuri[stanga_jos].y,
                                 k_diag, d_diag, spatiere_diag);
        S.adaugaArcuri(aDiag2);
    }
}

// 6. Pregatire finala
S.seteazaCoeficientFrecare(0.2);
S.seteazaCoeficientRestituire(0.8);
S.incarcaStare();
S.seteazaMatriceInertie();

S.corpuri[S.id_corp_mouse].collider.culoare.r = 1.0f;
S.corpuri[S.id_corp_mouse].collider.culoare.g = 0.0f;
S.corpuri[S.id_corp_mouse].collider.culoare.b = 0.0f;
S.corpuri[S.id_corp_mouse].collider.culoare.a = 1.0f;


    // Initializare OpenGL
    std::cout << "==> Deschidere Fereastra OpenGL..." << std::endl;
    unsigned int shaderProgram;
    GLFWwindow* window = openGLWindow(shaderProgram);

    if(window != NULL){
        // Initializare Buffere (VAO, VBO)
        unsigned int VAO, VBO;
        initBuffers(VAO, VBO);

        // Buffer local pentru coordonate
        // 10 valori (x, y, phi, w, h, type, red, green, blue, alpha) * (nr_corpuri + nr_legaturi)
        std::vector<float> vertexBuffer(10 * (S.nr_corpuri + S.nr_legaturi + S.arcuri.size()));

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
            if(frameCount % 100 == 0 && running_flag == 1){
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