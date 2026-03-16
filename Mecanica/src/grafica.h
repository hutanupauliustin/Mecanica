#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "sistem.h"

float zoomScale = 10.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset este +1 când dai scroll în sus (zoom in) și -1 când dai scroll în jos (zoom out)
    zoomScale -= (float)yoffset * 0.5f; // 0.5f este sensibilitatea (cât de rapid faci zoom)
    
    // Punem limite pentru a nu da zoom prea aproape și pentru a evita împărțirea la 0 (sau numere negative care întorc ecranul cu capul în jos)
    if (zoomScale < 1.0f) 
        zoomScale = 1.0f; 
    if (zoomScale > 100.0f) 
        zoomScale = 100.0f; 
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)   //modifica marimea ecranului cand se modifica marinea ferestrei
{
        glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float &dt, bool &running_flag) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Accelerare
    static bool plusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        if(!plusApasat){
            dt += 0.001f; 
            if(dt > 0.01f) dt = 0.01f;
            plusApasat = true;
        }
    } else {
        // Se reseteaza abia cand ridici degetul de pe tasta
        plusApasat = false;
    }

    // Decelerare
    static bool minusApasat = false;
    if(glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        if(!minusApasat){
            dt -= 0.001f; 
            if(dt < 0.0001f) dt = 0.0001f;
            minusApasat = true;
        }
    } else {
        minusApasat = false;
    }

    // Pauza
    static bool spaceApasat = false;
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        if(!spaceApasat) {
            running_flag = !running_flag; 
            spaceApasat = true;
        }
    } else {
        spaceApasat = false; 
    }
}

// --- 1. VERTEX SHADER: Primeste datele si le da mai departe la Geometry Shader ---
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in float aPhi;\n"  // Unghiul de rotatie
    "layout (location = 2) in vec2 aSize;\n"  // Dimensiuni (width, height)
    "layout (location = 3) in float aType;\n" // Tipul formei: 0=Invizibil,1=Cerc, 2=Dreptunghi, 3=Triunghi
    
    "out VS_OUT {\n"
    "    float phi;\n"
    "    vec2 size;\n"
    "    float type;\n"
    "} vs_out;\n"
    
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "   vs_out.phi = aPhi;\n"
    "   vs_out.size = aSize;\n"
    "   vs_out.type = aType;\n"
    "}\0";

// --- 2. GEOMETRY SHADER: Transforma punctele in forme geometrice ---  // scris de AI
const char *geometryShaderSource = "#version 330 core\n"
    "layout (points) in;\n"
    "layout (triangle_strip, max_vertices = 4) out;\n"
    "in VS_OUT {\n"
    "    float phi;\n"
    "    vec2 size;\n"
    "    float type;\n"
    "} gs_in[];\n"
    "out vec3 TexCoord;\n" // x, y locale si z = l_0
    "out float ShapeType;\n"
    "uniform float scale;\n"
    "void main() {\n"
    "    if(gs_in[0].type < 0.5) return;\n"
    "    float phi = gs_in[0].phi;\n"
    "    float c = cos(phi); float s = sin(phi);\n"
    "    mat2 rot = mat2(c, s, -s, c);\n"
    "    vec2 center = gl_in[0].gl_Position.xy;\n"
    "    ShapeType = gs_in[0].type;\n"
    "    vec2 halfSize;\n"
    "    if(ShapeType > 3.5) halfSize = vec2(gs_in[0].size.x / 2.0, 0.15);\n"
    "    else halfSize = gs_in[0].size / 2.0;\n"
    "    vec2 offsets[4];\n"
    "    offsets[0] = vec2(-halfSize.x, -halfSize.y);\n"
    "    offsets[1] = vec2( halfSize.x, -halfSize.y);\n"
    "    offsets[2] = vec2(-halfSize.x,  halfSize.y);\n"
    "    offsets[3] = vec2( halfSize.x,  halfSize.y);\n"
    "    vec2 uvs[4];\n"
    "    uvs[0] = vec2(-1.0, -1.0); uvs[1] = vec2(1.0, -1.0);\n"
    "    uvs[2] = vec2(-1.0, 1.0);  uvs[3] = vec2(1.0, 1.0);\n"
    "    for(int i=0; i<4; i++) {\n"
    "        vec2 pos = center + rot * offsets[i];\n"
    "        gl_Position = vec4(pos * scale, 0.0, 1.0);\n"
    "        TexCoord = vec3(uvs[i], gs_in[0].size.y);\n" // size.y este l_0
    "        EmitVertex();\n"
    "    }\n"
    "    EndPrimitive();\n"
    "}\0";

// --- 3. FRAGMENT SHADER ---      
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 TexCoord;\n"
    "in float ShapeType;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    // Cerc: folosim x si y din TexCoord (primele doua componente)
    "    if(ShapeType < 1.5 && dot(TexCoord.xy, TexCoord.xy) > 1.0) discard;\n"
    "    FragColor = color;\n"
    "    if(ShapeType > 3.5) {\n"
    "        float l_0 = TexCoord.z;\n"
    "        float spirePerMetru = 8.0;\n"
    "        float spire = l_0 * spirePerMetru;\n"
    "        float unda = sin(TexCoord.x * 3.1415 * spire);\n"
    // Atenuare cu parabola (inmultire, nu adunare)
    "        unda *= (1.0 - TexCoord.x * TexCoord.x);\n"
    // Verificam distanta pixelului pe Y local (TexCoord.y) fata de unda
    "        if(abs(TexCoord.y - unda) > 0.35) discard;\n"
    "    }\n"
    "}\0";

void updateVerticesData(sistem &S, float* vertices){
    // Structura datelor per punct: [x, y, phi, width, height, type] (6 float-uri)
    // type : 1 -- cerc 2 -- dreptunghi 4 --arc
    int stride = 6;

    // 1. Corpuri
    for(int i = 0; i < S.nr_corpuri; i++){
        int idx = i * stride;
        vertices[idx + 0] = S.corpuri[i].x;
        vertices[idx + 1] = S.corpuri[i].y;
        vertices[idx + 2] = S.corpuri[i].phi;

        if(S.corpuri[i].collider.tip == CERC){
            vertices[idx + 3] = S.corpuri[i].collider.dimensiune1 * 2.0f; // Shaderul asteapta Diametru, dar noi stocam Raza
            vertices[idx + 4] = S.corpuri[i].collider.dimensiune2 * 2.0f;
        } else {
            vertices[idx + 3] = S.corpuri[i].collider.dimensiune1;
            vertices[idx + 4] = S.corpuri[i].collider.dimensiune2;
        }
        vertices[idx + 5] = (float)S.corpuri[i].collider.tip;
    }

    // 2. Legaturi 
    int offset = S.nr_corpuri * stride;
    for(int i = 0; i <  S.nr_legaturi;i++){
        int idx = offset + i * stride;
        vertices[idx + 0] = S.legaturi[i]->getAbscisa(S.stare);
        vertices[idx + 1] = S.legaturi[i]->getOrdonata(S.stare); 
        float widht, height, phi;
        int type;
        S.legaturi[i]->getGraphics(S.stare,type, widht, height, phi);  // le schimba prin referinta
        vertices[idx + 2] = phi; // Phi
        vertices[idx + 3] = widht; // Width (Diametru)
        vertices[idx + 4] = height; // Height
        vertices[idx + 5] = type; // Type 1 = Cerc
    }

    //3. Arcuri

    offset = (S.nr_corpuri + S.nr_legaturi)*stride;
    for(int i = 0; i <  S.arcuri.size();i++){
        int idx = offset + i * stride;

        float x1, y1, x2, y2;
        S.corpuri[S.arcuri[i].contorCorpA].coordPunctPeCorp(x1, y1, S.arcuri[i].l_xA, S.arcuri[i].l_yA);
        S.corpuri[S.arcuri[i].contorCorpB].coordPunctPeCorp(x2, y2, S.arcuri[i].l_xB, S.arcuri[i].l_yB);

        float dx = x2 - x1;
        float dy = y2 - y1;
        float phi = std::atan2(y2 - y1, x2 - x1);
        float lungime_curenta = std::sqrt(dx * dx + dy * dy);
        float lungime_repaus = S.arcuri[i].lungime_0;

        vertices[idx + 0] = (x1 + x2 )/ 2.0f;
        vertices[idx + 1] = (y1 + y2 )/ 2.0f;
        vertices[idx + 2] = phi;

        vertices[idx + 3] = lungime_curenta; 
        vertices[idx + 4] = lungime_repaus;       
        vertices[idx + 5] = 4.0f;
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
    glfwSwapInterval(1); // Activeaza V-Sync (limiteaza la 60 FPS) pentru a nu rula simularea prea repede
    glfwSetScrollCallback(window, scroll_callback);

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

    // Compilare Geometry Shader
    unsigned int geometryShader;
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    glCompileShader(geometryShader);
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);               // placa video ii da un ID shader-ului
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);    // trimite textul placii video, ca sa il compileze
    glCompileShader(fragmentShader);                                   // ii spune placii video sa ruleze shader-ul (codul)

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);    
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);                                     //program este un obiect care are rolul de a lega shaderele intre ele 

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
         std::cout << "ERROR::PROGRAM::FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader); 

    return window;
}

void initBuffers(unsigned int &VAO, unsigned int &VBO) {
    glGenVertexArrays(1, &VAO);   // ii spune placii video sa le creeze
    glGenBuffers(1, &VBO);   
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    
    // Avem acum 6 float-uri per vertex: x, y, phi, w, h, type
    int stride = 6 * sizeof(float);

    // 1. Pozitie (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // 2. Phi (float)
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 3. Size (vec2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // 4. Type (float)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
}


void drawSystem(sistem &S, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer) {
    // 1. Actualizam datele in RAM
    updateVerticesData(S, Buffer);
    int totalPoints = S.nr_corpuri + S.nr_legaturi + S.arcuri.size();

    // 2. Trimitem datele la GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, totalPoints * 6 * sizeof(float), Buffer, GL_DYNAMIC_DRAW);
    
    // 3. Desenam
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glPointSize(1.0f); // Desenam puncte mari sa se vada

    // Setam factorul de scalare
    int scaleVertexLoc = glGetUniformLocation(shaderProgram, "scale");
    glUniform1f(scaleVertexLoc, 1.0f / zoomScale);
    
    // Obtinem locatia variabilei 'color' din shader
    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    // 1. Desenam Corpurile (Roz) - de la index 0, atatea cate corpuri sunt
    glUniform4f(colorLoc, 1.0f, 0.0f, 0.4f, 1.0f);
    glDrawArrays(GL_POINTS, 0, S.nr_corpuri);

    // 2. Desenam Legaturile (Alb) - incepand de unde s-au terminat corpurile
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_POINTS, S.nr_corpuri, S.nr_legaturi);

    // 3. Desenam Arcurile (Portocaliu) 
    glUniform4f(colorLoc, 1.0f, 0.61f, 0.0f, 1.0f);
    glDrawArrays(GL_POINTS, S.nr_legaturi + S.nr_corpuri, S.arcuri.size());

    glBindVertexArray(0);


}
