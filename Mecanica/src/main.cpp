#include "sistem.h"
#include "grafica.h"
#include "fizica.h" // Includem header-ul, nu fisierul sursa

// Constanta pentru conversia grade -> radiani
const float DEG_TO_RAD = 3.1415926535f / 180.0f;

int main(){
    sistem S(2,1);

    legatura* art = new articulatie(0, 1, 2, 1, 2, 1);
    rigid bara1(3, 0, 30.0f * DEG_TO_RAD, 4, 4 , 1);
    rigid bara2(5, 0, -30.0f * DEG_TO_RAD, 4, 4, 0); 

    S.adaugaCorpuri(bara1);
    S.adaugaCorpuri(bara2);
    S.adaugaLegaturi(art);

    // Initializam starea sistemului si matricea de inertie
    S.incarcaStare();
    S.seteazaMatriceInertie();

    // Initializare OpenGL
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
        while(!glfwWindowShouldClose(window)) {
            // 1. Input
            processInput(window);

            // 2. Fizica (RK4)
            S.stare = RK4(S, dt, t);
            S.seteazaStare(); // Actualizam obiectele rigid din matricea stare
            t += dt;

            // 3. Randare
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            drawSystem(S, VAO, VBO, shaderProgram, vertexBuffer);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        delete[] vertexBuffer;
        glfwTerminate();
    }
    return 0;
}