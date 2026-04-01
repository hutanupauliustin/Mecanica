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
    "layout (location = 5) in float aFiltru;\n"
    "layout (location = 6) in vec3 aVel;\n"
    "layout (location = 7) in vec3 aAcc;\n"

    "out VS_OUT {\n"
    "    float phi;\n"
    "    vec2 size;\n"
    "    float type;\n"
    "    vec4 color;\n"
    "    float filtru;\n"
    "    vec3 vVel;\n" 
    "    vec3 vAcc;\n"
    "} vs_out;\n"

    "void main() {\n"
    "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "   vs_out.phi = aPhi;\n"
    "   vs_out.size = aSize;\n"
    "   vs_out.type = aType;\n"
    "   vs_out.color = aColor;\n"
    "   vs_out.filtru = aFiltru;\n"
    "   vs_out.vVel = aVel;\n" 
    "   vs_out.vAcc = aAcc;\n"
    "}\0";

// --- 2. GEOMETRY SHADER ---  
const char *geometryShaderSource = "#version 330 core\n"
    "layout (points) in;\n"
    "layout (triangle_strip, max_vertices = 4) out;\n"

    "in VS_OUT {\n"
    "    float phi; vec2 size; float type; vec4 color; float filtru; vec3 vVel; vec3 vAcc;\n"
    "} gs_in[];\n"

    "out vec3 TexCoord;\n"
    "out float ShapeType;\n"
    "out vec4 fColor;\n"
    "out float filtru;\n"
    "out vec3 fVel; out vec3 fAcc; out vec2 fLocalPos;\n" // Noile iesiri

    "uniform float scale; uniform vec2 cameraOffset; uniform float aspect_ratio;\n"

    "void main() {\n"
    "    if(gs_in[0].type < 0.5) return;\n"
    "    mat2 rot = mat2(cos(gs_in[0].phi), sin(gs_in[0].phi), -sin(gs_in[0].phi), cos(gs_in[0].phi));\n"
    "    vec2 center = gl_in[0].gl_Position.xy - cameraOffset;\n"
    
    "    ShapeType = gs_in[0].type;\n"
    "    fColor = gs_in[0].color;\n"
    "    filtru = gs_in[0].filtru;\n"
    "    fVel = gs_in[0].vVel; fAcc = gs_in[0].vAcc;\n"

    "    float expand = (filtru > 0.5 && ShapeType < 3.5) ? 1.15 : 1.0;\n"
    "    vec2 halfSize = (ShapeType > 3.5 && ShapeType < 4.5) ? vec2(gs_in[0].size.x / 2.0, 0.15) : (gs_in[0].size / 2.0) * expand;\n"

    "    vec2 offsets[4] = vec2[](vec2(-halfSize.x, -halfSize.y), vec2(halfSize.x, -halfSize.y), vec2(-halfSize.x, halfSize.y), vec2(halfSize.x, halfSize.y));\n"
    "    vec2 uvs[4] = vec2[](vec2(-1.0, -1.0)*expand, vec2(1.0, -1.0)*expand, vec2(-1.0, 1.0)*expand, vec2(1.0, 1.0)*expand);\n"

    "    for(int i=0; i<4; i++) {\n"
    "        fLocalPos = offsets[i] / expand;\n" // Vectorul r (metri) fara expansiunea de glow
    "        vec2 pos = center + rot * offsets[i];\n"
    "        gl_Position = vec4((pos.x * scale)/aspect_ratio, pos.y * scale, 0.0, 1.0);\n"
    "        TexCoord = vec3(uvs[i], gs_in[0].size.y);\n"
    "        EmitVertex();\n"
    "    }\n"
    "    EndPrimitive();\n"
    "}\0";

// --- 3. FRAGMENT SHADER ---      
    const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 TexCoord; in float ShapeType; in vec4 fColor; in float filtru;\n"
    "in vec3 fVel; in vec3 fAcc; in vec2 fLocalPos;\n"
    
    "uniform int ViewMode;\n"

    "void main() {\n"
    "   vec4 outputColor = fColor;\n"
    "   bool inBorder = false;\n"
    "    bool isHovered = (filtru > 0.5 && filtru < 1.5) || (filtru > 2.5);\n" // E sub mouse
    "    bool isSelected = (filtru > 1.5);\n" // E selectat din click
    //
    "    float raza_curenta = isHovered ? 1.06 : 1.0;\n" 
    "    float grosime_margine = 0.08;\n" // Cat de groasa e linia alba
    //
        // --- DECUPARE FORME (DISCARD) --- 
    "    if(ShapeType < 1.5) {\n" // CERC
    "        float d = length(TexCoord.xy);\n"
    "        if (d > raza_curenta + (isSelected ? grosime_margine : 0.0)) discard;\n" // Taiem ce e in afara
    
    "        if (isSelected && d > raza_curenta) {\n"
    "            outputColor = vec4(1.0, 1.0, 1.0, 1.0); // Alb pentru margine\n"
    "            inBorder = true;\n"
    "        }\n"
    "    }\n"
    "    else if (ShapeType > 1.5 && ShapeType < 3.5 ) {\n" // DREPTUNGHI
    "        float d = max(abs(TexCoord.x), abs(TexCoord.y));\n"
    "        if (d > raza_curenta + (isSelected ? grosime_margine : 0.0)) discard;\n"
    "        if (isSelected && d > raza_curenta) {\n"
    "            outputColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "            inBorder = true;\n"
    "        }\n"
    "    }\n"
    "    else if (ShapeType > 3.5 && ShapeType < 4.5) {\n" // ARC
    "        float l_0 = TexCoord.z;\n"
    "        float spire = l_0 * 8.0;\n"
    "        float unda = sin(TexCoord.x * 3.1415 * spire) * (1.0 - TexCoord.x * TexCoord.x);\n"
    "        if(abs(TexCoord.y - unda) > 0.35) discard;\n"
    "    }\n"
    "    else if (ShapeType > 4.5 && ShapeType < 5.5){\n" // SAGEATA
    "        float x = TexCoord.x;\n"
    "        float y = abs(TexCoord.y);\n"
    "        float start_varf = 0.5;\n"
    "        float grosime_coada = 0.35;\n"
    "        if(x < start_varf){\n"
    "            if(y > grosime_coada) discard;\n"
    "        } else {\n"
    "            float factor_panta = (x - start_varf) / (1.0 - start_varf);\n"
    "            if(y > (1.0 - factor_panta)) discard;\n"
    "        }\n"
    "    }\n"

        // --- VIZUALIZARE STANDARD --- 
    "    if (ViewMode == 0 || ShapeType > 3.5) {\n"
    "        FragColor =outputColor;\n"
    "        if(filtru > 1.5f && filtru < 2.5f){\n}"
    "           return;\n"
    "    }\n"

    "    // --- HEATMAP (VITEZE SAU ACCELERATII) --- \n"
    "    float mag = 0.0;\n"
    "    if (ViewMode == 1) {\n" // vM = vO + w x r
    "        vec2 vM = fVel.xy + vec2(-fVel.z * fLocalPos.y, fVel.z * fLocalPos.x);\n"
    "        mag = length(vM);\n"
    "    } else if (ViewMode == 2) {\n" // aM = aO + eps x r - w^2 * r
    "        vec2 a_tang = vec2(-fAcc.z * fLocalPos.y, fAcc.z * fLocalPos.x);\n"
    "        vec2 a_norm = -(fVel.z * fVel.z) * fLocalPos;\n"
    "        mag = length(fAcc.xy + a_tang + a_norm);\n"
    "    }\n"

    "    float t = clamp(mag / (ViewMode == 1 ? 20.0 : 150.0), 0.0, 1.0);\n"
    "    vec3 heat = (t < 0.5) ? mix(vec3(0.1, 0.3, 0.8), vec3(1.0, 0.8, 0.0), t*2.0) : mix(vec3(1.0, 0.8, 0.0), vec3(1.0, 0.0, 0.0), (t-0.5)*2.0);\n"
    "    if (inBorder) {\n"
    "        FragColor = vec4(1.0, 1.0, 1.0, outputColor.a);\n"
    "    } else {\n"
    "        FragColor = vec4(heat, outputColor.a);\n"
    "    }\n"
    "}\0";

int updateVerticesData(sistem &S, editor &E, float* vertices){
    // 17 float-uri: [x, y, phi, width, height, type, r, g, b, a, selectie, viteza_x, viteza_y, omega, acc_x, acc_y, epsilon]
    int stride = 17;
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
        rigid &r = S.corpuri[i];
        
        if(r.activ == 0){
            for(int k=0; k<11; k++) vertices[idx + k] = 0;
            pct_curent++;
            continue;
        }

        vertices[idx + 0] = r.pozitie.x;
        vertices[idx + 1] = r.pozitie.y;
        vertices[idx + 2] = r.phi;

        if(r.collider.tip == CERC){
            vertices[idx + 3] = r.collider.dimensiune1 * 2.0f; 
            vertices[idx + 4] = r.collider.dimensiune2 * 2.0f;
        } else {
            vertices[idx + 3] = r.collider.dimensiune1;
            vertices[idx + 4] = r.collider.dimensiune2;
        }

        float alpha = r.collider.culoare.a;
        if (E.mod_curent == 1 && r.collider.cadru != E.cadru_activ && r.collider.obiectVirtual == 0) {
            alpha *= 0.2f; 
        }

        vertices[idx + 5] = (float)r.collider.tip;
        vertices[idx + 6] = (float)r.collider.culoare.r;
        vertices[idx + 7] = (float)r.collider.culoare.g;
        vertices[idx + 8] = (float)r.collider.culoare.b;
        vertices[idx + 9] = alpha;
        
        float stare_filtru = 0.0f;
        if (r.collider.subMouse) stare_filtru += 1.0f; // Adaugă 1 dacă e sub mouse
        if (r.collider.selectat) stare_filtru += 2.0f; // Adaugă 2 dacă e selectat
        
        vertices[idx + 10] = stare_filtru;

        vertices[idx + 11] = r.viteza.x;
        vertices[idx + 12] = r.viteza.y;
        vertices[idx + 13] = r.omega;
        vertices[idx + 14] = r.forte_desen.acc_cadru.x;
        vertices[idx + 15] = r.forte_desen.acc_cadru.y;
        vertices[idx + 16] = r.forte_desen.eps_cadru;
        
        pct_curent++;
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

        vertices[idx + 11] = 0;
        vertices[idx + 12] = 0;
        vertices[idx + 13] = 0;
        vertices[idx + 14] = 0;
        vertices[idx + 15] = 0;
        vertices[idx + 16] = 0;
        
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

        vertices[idx + 11] = 0;
        vertices[idx + 12] = 0;
        vertices[idx + 13] = 0;
        vertices[idx + 14] = 0;
        vertices[idx + 15] = 0;
        vertices[idx + 16] = 0;
        
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
    if (E.flag.arata_forte) {
        for (int i = 0; i < S.corpuri.size(); i++) {
            if (!S.corpuri[i].activ || S.corpuri[i].M > 1e10f) continue;

            float scala_forta = 0.02f;

            auto adaugaSageata = [&](vec2 forta, vec2 origine,  float r, float g, float b) {
                    
                    float magnitudine = forta.modul(); 
                    if (magnitudine < 0.1f) return;

                    int idx = pct_curent * stride;
                    float unghi = std::atan2(forta.y, forta.x);
                    float L = std::log10(1.0f + magnitudine) * 0.4f;
                    float grosime = 0.15f + (L *0.1f);

                    vertices[idx + 0] = origine.x + (L / 2.0f) * std::cos(unghi);
                    vertices[idx + 1] = origine.y + (L / 2.0f) * std::sin(unghi);
                    vertices[idx + 2] = unghi;
                    vertices[idx + 3] = L; 
                    vertices[idx + 4] = grosime; // grosimea sagetii
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
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

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
    
    GLsizei stride = 17 * sizeof(float);

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
    // 7. viteza (vx, vy, omega)
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(6);
    // 8. Accelerația ( ax, ay, eps)
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, stride, (void*)(14 * sizeof(float)));
    glEnableVertexAttribArray(7);

    glBindVertexArray(0);
}

void drawSystem(sistem &S, editor &E, unsigned int VAO, unsigned int VBO, unsigned int shaderProgram, float* Buffer) {

    int totalPoints = updateVerticesData(S, E, Buffer);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, totalPoints * 17 * sizeof(float), Buffer, GL_DYNAMIC_DRAW);
    
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glPointSize(1.0f); 

    int scaleVertexLoc = glGetUniformLocation(shaderProgram, "scale");
    glUniform1f(scaleVertexLoc, 1.0f / zoomScale);

    int camOffsetLoc = glGetUniformLocation(shaderProgram, "cameraOffset");
    glUniform2f(camOffsetLoc, cameraX, cameraY);

    int aspectLoc = glGetUniformLocation(shaderProgram, "aspect_ratio");
    glUniform1f(aspectLoc, aspect_ratio);

    int viewModeLoc = glGetUniformLocation(shaderProgram, "ViewMode");
    glUniform1i(viewModeLoc,E.mod_vizualizare);

    glDrawArrays(GL_POINTS, 0, totalPoints);
    glBindVertexArray(0);
}