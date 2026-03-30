#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "input.h"
#include "grafica.h"
#include "editor.h"

// --- 1. VERTEX SHADER ---
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in float aPhi;\n"  
    "layout (location = 2) in vec2 aSize;\n"  
    "layout (location = 3) in float aType;\n" 
    "layout (location = 4) in vec4 aColor;\n" 
    "layout (location = 5) in float aSelected;\n" // Flag-ul pentru selecție

    "out VS_OUT {\n"
    "    float phi;\n"
    "    vec2 size;\n"
    "    float type;\n"
    "    vec4 color;\n"
    "    float isSelected;\n"
    "} vs_out;\n"
    
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "   vs_out.phi = aPhi;\n"
    "   vs_out.size = aSize;\n"
    "   vs_out.type = aType;\n"
    "   vs_out.color = aColor;\n"
    "   vs_out.isSelected = aSelected;\n" // Am pus punctul si virgula lipsa
    "}\0";

// --- 2. GEOMETRY SHADER ---  
const char *geometryShaderSource = "#version 330 core\n"
    "layout (points) in;\n"
    "layout (triangle_strip, max_vertices = 4) out;\n"

    "in VS_OUT {\n"
    "    float phi;\n"
    "    vec2 size;\n"
    "    float type;\n"
    "    vec4 color;\n"
    "    float isSelected;\n"
    "} gs_in[];\n"

    "out vec3 TexCoord;\n" 
    "out float ShapeType;\n"
    "out vec4 fColor;\n"
    "out float fSelected;\n" // Dam mai departe spre Fragment Shader

    "uniform float scale;\n"
    "uniform vec2 cameraOffset;\n"
    "uniform float aspect_ratio;\n"
    
    "void main() {\n"
    "    if(gs_in[0].type < 0.5) return;\n"
    "    float phi = gs_in[0].phi;\n"
    "    float c = cos(phi); float s = sin(phi);\n"
    "    mat2 rot = mat2(c, s, -s, c);\n"
    "    vec2 center = gl_in[0].gl_Position.xy - cameraOffset;\n"
    "    ShapeType = gs_in[0].type;\n"
    "    fColor = gs_in[0].color;\n"
    "    fSelected = gs_in[0].isSelected;\n"
    
    // Extindem geometria cu 20% daca e un corp (nu arc) si e selectat
    "    float expand = (fSelected > 0.5 && ShapeType < 3.5) ? 1.02 : 1.0;\n"
    
    "    vec2 halfSize;\n"
    "    if(ShapeType > 3.5 && ShapeType < 4.5) halfSize = vec2(gs_in[0].size.x / 2.0, 0.15);\n"
    "    else halfSize = (gs_in[0].size / 2.0) * expand;\n" // Aplicam expansiunea pe X si Y
    
    "    vec2 offsets[4];\n"
    "    offsets[0] = vec2(-halfSize.x, -halfSize.y);\n"
    "    offsets[1] = vec2( halfSize.x, -halfSize.y);\n"
    "    offsets[2] = vec2(-halfSize.x,  halfSize.y);\n"
    "    offsets[3] = vec2( halfSize.x,  halfSize.y);\n"
    
    "    vec2 uvs[4];\n" // UV-urile se scaleaza si ele pentru a pastra d=1 la marginea reala
    "    uvs[0] = vec2(-1.0, -1.0) * expand; uvs[1] = vec2(1.0, -1.0) * expand;\n"
    "    uvs[2] = vec2(-1.0, 1.0) * expand;  uvs[3] = vec2(1.0, 1.0) * expand;\n"
    
    "    for(int i=0; i<4; i++) {\n"
    "        vec2 pos = center + rot * offsets[i];\n"
    "        gl_Position = vec4((pos.x * scale )/ aspect_ratio ,pos.y * scale, 0.0, 1.0);\n"
    "        TexCoord = vec3(uvs[i], gs_in[0].size.y);\n"
    "        EmitVertex();\n"
    "    }\n"
    "    EndPrimitive();\n"
    "}\0";

// --- 3. FRAGMENT SHADER ---      
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 TexCoord;\n"
    "in float ShapeType;\n"
    "in vec4 fColor;\n"
    "in float fSelected;\n"
    
    "void main() {\n"
    "    vec3 finalGlow = vec3(0.0);\n"
    
    "    if(ShapeType < 1.5) {\n" // CERC
    "        float d = dot(TexCoord.xy, TexCoord.xy);\n"
    "        if(fSelected > 0.5) {\n"
    "            if (d > 1.44) discard;\n" // 1.2^2 = 1.44
    "            float glow = clamp((1.44 - d) / 0.44, 0.0, 1.0);\n"
    "            finalGlow = vec3(1.0) * pow(glow, 2.0) * 0.4;\n"
    "        } else if (d > 1.0) discard;\n"
    "    }\n"
    "    else if (ShapeType < 3.5) {\n" // DREPTUNGHI 
    "        float d = max(abs(TexCoord.x), abs(TexCoord.y));\n"
    "        if(fSelected > 0.5) {\n"
    "            if (d > 1.2) discard;\n"
    "            float glow = clamp((1.2 - d) / 0.2, 0.0, 1.0);\n"
    "            finalGlow = vec3(1.0) * pow(glow, 2.0) * 0.4;\n"
    "        } else if (d > 1.0) discard;\n"
    "    }\n"
    "    else if (ShapeType > 3.5 && ShapeType < 4.5) {\n" // ARCURI cu unda sinus
    "        float l_0 = TexCoord.z;\n"
    "        float spirePerMetru = 8.0;\n"
    "        float spire = l_0 * spirePerMetru;\n"
    "        float unda = sin(TexCoord.x * 3.1415 * spire);\n"
    "        unda *= (1.0 - TexCoord.x * TexCoord.x);\n"
    "        if(abs(TexCoord.y - unda) > 0.35) discard;\n"
    "    }\n"
    "    else if(ShapeType > 4.5 && ShapeType < 5.5){\n "        //SAGEATA
    "       float x = TexCoord.x;\n"
    "       float y = abs(TexCoord.y);\n"
    "       float start_varf = 0.4;\n"
    "       float grosime_coada =0.25;\n"

    "       if(x < start_varf){\n"
    "           if(y > grosime_coada) discard;\n"      
    "       } else {\n"
    "           float panta = (1.0 - x) / (1.0 - start_varf);\n"
    "           if(y> panta) discard;\n"
    "       }\n"
    "    }\n"
    "    FragColor = vec4(fColor.rgb + finalGlow, fColor.a);\n"
    "}\0";

int updateVerticesData(sistem &S, editor &E, float* vertices, bool arata_forte){
    // 11 float-uri: [x, y, phi, width, height, type, r, g, b, a, selectat]
    int stride = 11;
    int pct_curent = 0; // Contor global pentru pozitia in buffer

    // Gasim ordinea corecta pentru corpuri (Painter's Algorithm)
    std::vector<int> ordine_corpuri;
    for (int i = 0; i < S.corpuri.size(); i++) {
        ordine_corpuri.push_back(i);
    }

    std::sort(ordine_corpuri.begin(), ordine_corpuri.end(), [&S](int a, int b) {
        return S.corpuri[a].collider.cadru < S.corpuri[b].collider.cadru;
    });

    // 1. Corpuri
    for(int pos = 0; pos < ordine_corpuri.size(); pos++){
        int i  = ordine_corpuri[pos];
        int idx = pct_curent * stride;
        
        if(S.corpuri[i].activ == 0){
            for(int k=0; k<11; k++) vertices[idx + k] = 0;
            pct_curent++; // FOARTE IMPORTANT: Avansam in memorie si pt cele inactive!
            continue;
        }

        vertices[idx + 0] = S.corpuri[i].pozitie.x;
        vertices[idx + 1] = S.corpuri[i].pozitie.y;
        vertices[idx + 2] = S.corpuri[i].phi;

        if(S.corpuri[i].collider.tip == CERC){
            vertices[idx + 3] = S.corpuri[i].collider.dimensiune1 * 2.0f; 
            vertices[idx + 4] = S.corpuri[i].collider.dimensiune2 * 2.0f;
        } else {
            vertices[idx + 3] = S.corpuri[i].collider.dimensiune1;
            vertices[idx + 4] = S.corpuri[i].collider.dimensiune2;
        }

        float alpha = S.corpuri[i].collider.culoare.a;
        if (E.mod_curent == 1 && S.corpuri[i].collider.cadru != E.cadru_activ && S.corpuri[i].collider.obiectVirtual == 0) {
            alpha *= 0.2f; 
        }

        vertices[idx + 5] = (float)S.corpuri[i].collider.tip;
        vertices[idx + 6] = (float)S.corpuri[i].collider.culoare.r;
        vertices[idx + 7] = (float)S.corpuri[i].collider.culoare.g;
        vertices[idx + 8] = (float)S.corpuri[i].collider.culoare.b;
        vertices[idx + 9] = alpha;
        
        vertices[idx + 10] = S.corpuri[i].collider.selectat ? 1.0f : 0.0f;
        
        pct_curent++; // FOARTE IMPORTANT!
    }

    // 2. Legaturi 
    for(int i = 0; i <  S.legaturi.size(); i++){
        int idx = pct_curent * stride; // FARA OFFSET VECHI!

        if(S.legaturi[i]->activ == 0){
            for(int k=0; k<11; k++) vertices[idx + k] = 0;
            pct_curent++;
            continue;
        }

        vec2 poz = S.legaturi[i]->getPozitie(S.corpuri);

        vertices[idx + 0] = poz.x;
        vertices[idx + 1] = poz.y;
        float widht, height, phi;
        int type;
        S.legaturi[i]->getGraphics(S.stare,type, widht, height, phi); 
        vertices[idx + 2] = phi; 
        vertices[idx + 3] = widht; 
        vertices[idx + 4] = height; 
        vertices[idx + 5] = type; 
        vertices[idx + 6] = 1.0f;
        vertices[idx + 7] = 1.0f;
        vertices[idx + 8] = 1.0f;
        vertices[idx + 9] = 1.0f;
        
        vertices[idx + 10] = 0.0f; 
        
        pct_curent++;
    }

    //3. Arcuri
    for(int i = 0; i <  S.arcuri.size(); i++){
        int idx = pct_curent * stride; // FARA OFFSET VECHI!

        if(S.arcuri[i].activ == 0){
            for(int k=0; k<11; k++) vertices[idx + k] = 0;
            pct_curent++;
            continue;
        }

        vec2 p1 = S.corpuri[S.arcuri[i].contorCorpA].localToGlobal(S.arcuri[i].l_A);
        vec2 p2 = S.corpuri[S.arcuri[i].contorCorpB].localToGlobal(S.arcuri[i].l_B);
        
        vec2 diferenta = p2 - p1;
        float phi = std::atan2(diferenta.y , diferenta.x);
        float lungime_curenta = diferenta.modul();
        float lungime_repaus = S.arcuri[i].lungime_0;

        vertices[idx + 0] = (p1.x + p2.x )/ 2.0f;
        vertices[idx + 1] = (p1.y + p2.y )/ 2.0f;
        vertices[idx + 2] = phi;
        vertices[idx + 3] = lungime_curenta; 
        vertices[idx + 4] = lungime_repaus;       
        vertices[idx + 5] = 4.0f;
        vertices[idx + 6] = 1.0f;
        vertices[idx + 7] = 0.62f;
        vertices[idx + 8] = 0.0f;
        vertices[idx + 9] = 1.0f;
        
        vertices[idx + 10] = 0.0f;
        
        pct_curent++;
    }

    //4 Entitati de UI
    for(int i = 0; i < E.elementeUI.size(); i++){

        int idx = pct_curent * stride;

        if (!E.elementeUI[i].activa) {
        // Daca fantoma e oprita, punem 0 pe toata linia
        for(int k=0; k<11; k++) vertices[idx + k] = 0;
        pct_curent++;
        continue;
        }

        vertices[idx + 0] = E.elementeUI[i].x;
        vertices[idx + 1] = E.elementeUI[i].y;
        vertices[idx + 2] = E.elementeUI[i].phi;

       if(E.elementeUI[i].tip == CERC){
            vertices[idx + 3] = E.elementeUI[i].dim1 * 2.0f; 
            vertices[idx + 4] = E.elementeUI[i].dim2 * 2.0f;
        } else {
            vertices[idx + 3] = E.elementeUI[i].dim1;
            vertices[idx + 4] = E.elementeUI[i].dim2;
        }

        vertices[idx + 5] = (float)E.elementeUI[i].tip;
        vertices[idx + 6] = (float)E.elementeUI[i].col.r;
        vertices[idx + 7] = (float)E.elementeUI[i].col.g;
        vertices[idx + 8] = (float)E.elementeUI[i].col.b;
        vertices[idx + 9] = E.elementeUI[i].col.a;
        vertices[idx + 10] = 1;

        pct_curent++;
    }
   
    // 5. Forte (Desenate separat pe culori)
    if (arata_forte) {
        for (int i = 0; i < S.corpuri.size(); i++) {
            if (!S.corpuri[i].activ || S.corpuri[i].M > 1e10f) continue;

            float scala_forta = 0.02f;

            auto adaugaSageata = [&](vec2 forta, vec2 origine,  float r, float g, float b) {
                    
                    float magnitudine = forta.modul(); 
                    if (magnitudine < 0.1f) return;

                    int idx = pct_curent * stride;
                    float unghi = std::atan2(forta.y, forta.x);
                    float L = magnitudine * scala_forta;

                    vertices[idx + 0] = origine.x + (L / 2.0f) * std::cos(unghi);
                    vertices[idx + 1] = origine.y + (L / 2.0f) * std::sin(unghi);
                    vertices[idx + 2] = unghi;
                    vertices[idx + 3] = L; 
                    vertices[idx + 4] = L / 5; // grosimea sagetii
                    vertices[idx + 5] = 5.0f;  // Tip = SAGEATA
                    vertices[idx + 6] = r;
                    vertices[idx + 7] = g;
                    vertices[idx + 8] = b; 
                    vertices[idx + 9] = 0.8f; // Alpha
                    vertices[idx + 10] = 0.0f; // Selectat

                    pct_curent++;
                };

            for (const auto& f : S.corpuri[i].forte_desen.forte) {
                
                float r = 1.0f, g = 1.0f, b = 1.0f; // Alb default
                
                // Alegem culoarea in functie de tip
                switch (f.tip) {
                    case FORTA_GREUTATE:       r = 0.2f; g = 0.8f; b = 0.2f; break; // Verde
                    case FORTA_ELASTICA:       r = 0.2f; g = 0.5f; b = 1.0f; break; // Albastru
                    case FORTA_REACTIUNE:      r = 1.0f; g = 0.5f; b = 0.0f; break; // Portocaliu
                    case FORTA_IMPACT_NORMAL:  r = 0.0f; g = 1.0f; b = 1.0f; break; // Cyan
                    case FORTA_IMPACT_FRECARE: r = 1.0f; g = 1.0f; b = 0.0f; break; // Galben
                }

                // Trimitem forta.valoare si folosim f.punct_aplicare in loc de centrul corpului
                adaugaSageata(f.valoare, f.punct_aplicare, r, g, b); 
            }
        }
    }

    return pct_curent;
}


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

    glfwMakeContextCurrent(window);                                 
    glfwSwapInterval(1); 
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }    

    unsigned int vertexShader;  
    vertexShader = glCreateShader(GL_VERTEX_SHADER);  

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);     
    glCompileShader(vertexShader);                                  

    int  success;
    char infoLog[512];
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);       
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
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);               
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);    
    glCompileShader(fragmentShader);                                   

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);    
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);                                     

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
         std::cout << "ERROR::PROGRAM::FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader); 

    glEnable(GL_BLEND);                 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

void initBuffers(unsigned int &VAO, unsigned int &VBO) {
    glGenVertexArrays(1, &VAO);  
    glGenBuffers(1, &VBO);   
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    
    GLsizei stride = 11 * sizeof(float);

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
    // 5. Culoare (vec4)
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(4);
    // 6. isSelected (float) - CORECTAT AICI (Locatia 5, offset 10)
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(5);
    
    glBindVertexArray(0);
}

void drawSystem(sistem &S, editor &E, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer, bool arata_forte) {

    int totalPoints = updateVerticesData(S, E, Buffer, arata_forte);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, totalPoints * 11 * sizeof(float), Buffer, GL_DYNAMIC_DRAW);
    
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glPointSize(1.0f); 

    int scaleVertexLoc = glGetUniformLocation(shaderProgram, "scale");
    glUniform1f(scaleVertexLoc, 1.0f / zoomScale);

    int camOffsetLoc = glGetUniformLocation(shaderProgram, "cameraOffset");
    glUniform2f(camOffsetLoc, cameraX, cameraY);

    int aspectLoc = glGetUniformLocation(shaderProgram, "aspect_ratio");
    glUniform1f(aspectLoc, aspect_ratio);

    glDrawArrays(GL_POINTS, 0, totalPoints);
    glBindVertexArray(0);
}