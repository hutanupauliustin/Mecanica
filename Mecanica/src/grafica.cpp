#include "grafica.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }


void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
}

const char *vertexShaderSource = "#version 330 core\n"          //bucata de text, este cod pentru placa video 
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"

    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

int openGLWindow(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); 
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
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
    glCompileShader(fragmentShader);                                   // ii spune placii video sa ruleze codul

    unsigned int shaderProgram;
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

    GLuint VAO, VBO;  // Vertex Attribute Object ii spune placii video cum sa utilizeze datele din Vertex Buffer Object

    glGenVertexArrays(1, &VAO);   // ii spune placii video sa le creeze
    glGenBuffers(1, &VBO);   
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);  // aloca memoria pentru buffer

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);       // ii spune la VAO cum sa lucreze cu VBO
    glEnableVertexAttribArray(0);             
    
    glBindVertexArray(0);// 




}
