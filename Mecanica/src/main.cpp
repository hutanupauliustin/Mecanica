#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Funcție care se apelează automat când redimensionezi fereastra
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Funcție pentru a procesa input-ul de la tastatură
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // 1. Inițializăm GLFW
    glfwInit();
    
    // 2. Configurăm GLFW pentru OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Creăm fereastra
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simulator Mecanica 2D", NULL, NULL);
    if (window == NULL) {
        std::cout << "Eroare: Nu s-a putut crea fereastra GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 4. Inițializăm GLAD (încarcă pointerii către funcțiile OpenGL oferite de driverul tău video)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Eroare: Nu s-a putut inițializa GLAD" << std::endl;
        return -1;
    }

    // 5. Bucla principală de randare (Game Loop)
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Randare (aici vom desena solidele mai târziu)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Setăm culoarea de fundal (un albastru-verzui închis)
        glClear(GL_COLOR_BUFFER_BIT);         // Curățăm ecranul cu acea culoare

        // Swap buffers și poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 6. Curățăm resursele la închidere
    glfwTerminate();
    return 0;
}