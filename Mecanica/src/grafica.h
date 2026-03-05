#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)   //modifica marimea ecranului cand se modifica marinea ferestrei
    {
        glViewport(0, 0, width, height);
    }

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
}

const char *vertexShaderSource = "#version 330 core\n"          //bucata de text, este cod pentru placa video 
    "layout (location = 0) in vec2 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x / 5.0, aPos.y / 5.0, 0.0, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"

    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

void updateVerticesData(sistem &S, float* vertices){
    // 1. Adaugam coordonatele corpurilor (Centrele de Masa)
    for(int i = 0; i < S.nr_corpuri; i++){
        vertices[2*i] = S.corpuri[i].x;
        vertices[2*i + 1] = S.corpuri[i].y;
    }

    // 2. Adaugam coordonatele legaturilor (continuam in vector dupa corpuri)
    int offset = 2 * S.nr_corpuri;
    for(int i = 0; i <  S.nr_legaturi;i++){
        vertices[offset + 2*i] = S.legaturi[i]->getAbscisa(S.stare);
        vertices[offset + 2*i + 1] = S.legaturi[i]->getOrdonata(S.stare);
    }
}

// Modificam functia pentru a returna fereastra si a seta programul shader prin referinta
GLFWwindow* openGLWindow(unsigned int &shaderProgram){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "Proiect", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);                                 //specifica placii video ca lucram cu fereastra creata
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }    

    unsigned int vertexShader;  //ID pentru shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);  // ii spune placii video sa lase spatiu pt shader

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);     //trimite blocul de text in memoria placii video
    glCompileShader(vertexShader);                                  //ii spune placii video sa compileze textul ca fiind cod 

    int  success;
    char infoLog[512];
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);       //iv - integer vector
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);               // placa video ii da un ID shader-ului
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);    // trimite textul placii video, ca sa il compileze
    glCompileShader(fragmentShader);                                   // ii spune placii video sa ruleze shader-ul (codul)

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);    
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);                                     //program este un obiect care are rolul de a lega shaderele intre ele 

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
         std::cout << "ERROR::PROGRAM::FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); 

    return window;
}

void initBuffers(unsigned int &VAO, unsigned int &VBO) {
    glGenVertexArrays(1, &VAO);   // ii spune placii video sa le creeze
    glGenBuffers(1, &VBO);   
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);             
    
    glBindVertexArray(0);
}


void drawSystem(sistem &S, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer) {
    // 1. Actualizam datele in RAM
    updateVerticesData(S, Buffer);
    int totalPoints = S.nr_corpuri + S.nr_legaturi;

    // 2. Trimitem datele la GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, totalPoints * 2 * sizeof(float), Buffer, GL_DYNAMIC_DRAW);
    
    // 3. Desenam
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glPointSize(10.0f); // Desenam puncte mari sa se vada
    glDrawArrays(GL_POINTS, 0, totalPoints);
}
