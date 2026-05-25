#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "input.h"
#include "grafica.h"
#include "editor.h"
#include "instrument.h"

#include "shaders.h"

int updateVerticesData(sistem &S, editor &E, float* vertices){

    if(E.instrumentCurent) {
        E.instrumentCurent->pregatesteFantome(E.elementeUI, E.mouse_x, E.mouse_y, S);       //sterge cutia de selectie daca este cazul
    }

    // 17 float-uri: [x, y, phi, width, height, type, r, g, b, a, selectie, viteza_x, viteza_y, omega, acc_x, acc_y, epsilon]
    // tip 0 - punct 1- cerc 2-dreptunghi 3-triunghi
    int stride = 17;
    int pct_curent = 0; // Contor global pentru pozitia in buffer

    // Painter's Algorithm
    std::vector<int> ordine_corpuri;
    for (size_t i = 0; i < S.corpuri.size(); i++) {
        ordine_corpuri.push_back(i);
    }

    std::sort(ordine_corpuri.begin(), ordine_corpuri.end(), [&S](int a, int b) {
        return S.corpuri[a].collider.cadru < S.corpuri[b].collider.cadru;
    });

    // 1. Corpuri
    for(int pos = 0; pos < (int)ordine_corpuri.size(); pos++){
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
    for(size_t i = 0; i <  S.legaturi.size(); i++){
        int idx = pct_curent * stride; 

        if(S.legaturi[i]->activ == 0){
            for(int k=0; k<11; k++) vertices[idx + k] = 0;
            pct_curent++;
            continue;
        }

        vec2 poz = S.legaturi[i]->getPozitie(S.corpuri);
        float widht, height, phi;
        int type;
        float r = 1.0f, g = 1.0f, b = 1.0f, alpha =1.0f;
        S.legaturi[i]->getGraphics(S.stare,type, widht, height, phi,r,g,b,alpha); 

        float v_x = 0.0f, v_y = 0.0f, v_z = 0.0f;
        float a_x = 0.0f, a_y = 0.0f, a_z = 0.0f;

        // --- PRELUCRARE SPECIALA PENTRU FIRE ---
        if (fir* fir_ptr = dynamic_cast<fir*>(S.legaturi[i])) {
            type = 6;
            vec2 pA = S.corpuri[fir_ptr->contorCorpA].localToGlobal(fir_ptr->l_A);
            vec2 pB = S.corpuri[fir_ptr->contorCorpB].localToGlobal(fir_ptr->l_B);
            
            float burta = 0.0f;
            if (fir_ptr->tensionat) {
                r = 1.0f; g = 0.4f; b = 0.4f;
            } else {
                r = 0.8f; g = 0.8f; b = 0.8f;
                float L = (pB - pA).modul();
                float max_L = std::max(L, fir_ptr->lungime);
                // Deducem adancimea curbei din diferenta de lungime (Formula aproximativa)
                burta = std::sqrt(std::max(0.0f, 3.0f * L * (max_L - L) / 8.0f)) * 1.3f; 
            }

            // Calculam un bounding box FARA ROTATIE care cuprinde firul
            float padding = 0.3f;
            float minX = std::min(pA.x, pB.x) - padding;
            float maxX = std::max(pA.x, pB.x) + padding;
            // Scadem 'burta' din Y-ul minim, pentru ca gravitatia trage in jos (-Y)
            float minY = std::min(pA.y, pB.y) - burta - padding; 
            float maxY = std::max(pA.y, pB.y) + padding;

            poz.x = (minX + maxX) * 0.5f;
            poz.y = (minY + maxY) * 0.5f;
            widht = std::max(0.1f, maxX - minX);
            height = std::max(0.1f, maxY - minY);
            phi = 0.0f; // Ignoram rotatia! 

            // Trimitem punctele si burta spre placa video folosind atributele ramase libere
            v_x = pA.x; v_y = pA.y; v_z = burta;
            a_x = pB.x; a_y = pB.y;
        }

        vertices[idx + 0] = poz.x;
        vertices[idx + 1] = poz.y;
        vertices[idx + 2] = phi; 
        vertices[idx + 3] = widht; 
        vertices[idx + 4] = height; 
        vertices[idx + 5] = (float)type; 
        vertices[idx + 6] = r;
        vertices[idx + 7] = g;
        vertices[idx + 8] = b;
        vertices[idx + 9] = alpha; 

        float stare_filtru = 0.0f;
        if (S.legaturi[i]->subMouse) stare_filtru += 1.0f; 
        if (S.legaturi[i]->selectat) stare_filtru += 2.0f;
        
        vertices[idx + 10] = stare_filtru; 

        vertices[idx + 11] = v_x;
        vertices[idx + 12] = v_y;
        vertices[idx + 13] = v_z; 
        vertices[idx + 14] = a_x;
        vertices[idx + 15] = a_y;
        vertices[idx + 16] = 0.0f;
        
        pct_curent++;
    }

    //3. Generatori de forte
    for(size_t i = 0; i < S.surseForte.size(); i++){
        int idx = pct_curent * stride;

        int type = -1;
        float widht = 0.0f, height = 0.0f, phi = 0.0f;
        float r = 1.0f, g = 1.0f, b = 1.0f, alpha =1.0f;
        vec2 poz;

        S.surseForte[i]->getGraphics(S.stare, type, widht, height, phi, poz, r, g, b,alpha);

        if (type == -1  || !S.surseForte[i]->activ) {
            for(int k=0; k<11; k++) vertices[idx + k] = 0;
            pct_curent++;
            continue;
        }

        vertices[idx + 0] = poz.x;
        vertices[idx + 1] = poz.y;
        vertices[idx + 2] = phi;
        vertices[idx + 3] = widht; 
        vertices[idx + 4] = height;       
        vertices[idx + 5] = (float)type;
        vertices[idx + 6] = r;
        vertices[idx + 7] = g;
        vertices[idx + 8] = b;
        vertices[idx + 9] = alpha; // Alpha
        
        vertices[idx + 10] = 0.0f; // Filtru/Selectie

        vertices[idx + 11] = 0;
        vertices[idx + 12] = 0;
        vertices[idx + 13] = 0;
        vertices[idx + 14] = 0;
        vertices[idx + 15] = 0;
        vertices[idx + 16] = 0;
        
        pct_curent++;
    }

    //4 Entitati de UI
    for(size_t i = 0; i < E.elementeUI.size(); i++){

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
        for (size_t i = 0; i < S.corpuri.size(); i++) {
            if (!S.corpuri[i].activ || S.corpuri[i].M > 1e10f) continue;

            // --- 1. Lambda pentru sageti liniare (Tip 5) ---
            auto adaugaSageata = [&](vec2 forta, vec2 origine,  float r, float g, float b) {
                float magnitudine = forta.modul(); 
                if (magnitudine < 0.1f) return;

                int idx = pct_curent * E.vertexStride; 
                float unghi = std::atan2(forta.y, forta.x);
                float L = std::log10(1.0f + magnitudine) * 0.4f;
                float grosime = 0.15f + (L * 0.1f);

                vertices[idx + 0] = origine.x + (L / 2.0f) * std::cos(unghi);
                vertices[idx + 1] = origine.y + (L / 2.0f) * std::sin(unghi);
                vertices[idx + 2] = unghi;
                vertices[idx + 3] = L; 
                vertices[idx + 4] = grosime; // grosimea sagetii
                vertices[idx + 5] = 5.0f;  // Tip = SAGEATA LINIALA
                vertices[idx + 6] = r;
                vertices[idx + 7] = g;
                vertices[idx + 8] = b; 
                vertices[idx + 9] = 0.8f;  // Alpha
                vertices[idx + 10] = 0.0f; // Selectat

                pct_curent++;
            };

            // --- 2. Lambda pentru sageti rotative de moment (Tip 7) ---
            auto adaugaMoment = [&](float moment, vec2 origine, float r, float g, float b) {
                float magnitudine = std::abs(moment);
                if (magnitudine < 0.1f) return;

                int idx = pct_curent * E.vertexStride;
                
                // Determinam sensul: 1.0f pentru trigonometric (CCW), -1.0f pentru orar (CW)
                float sens = (moment > 0.0f) ? 1.0f : -1.0f; 
                
                // Calculam o raza vizuala pentru cerc (putin mai mica decat la forta ca sa nu acopere corpul)
                float raza = 0.05f + std::log10(1.0f + magnitudine) * 0.2f;
                float grosime = 0.05f + (raza * 0.1f);

                vertices[idx + 0] = origine.x;
                vertices[idx + 1] = origine.y;
                vertices[idx + 2] = sens;      // Aici trimitem SENSUL rotatiei in loc de unghi
                vertices[idx + 3] = raza;      // Folosim parametrul de lungime pentru raza cercului
                vertices[idx + 4] = grosime;   // Grosimea liniei
                vertices[idx + 5] = 7.0f;      // Tip = SAGEATA ROTATIVA
                vertices[idx + 6] = r;
                vertices[idx + 7] = g;
                vertices[idx + 8] = b; 
                vertices[idx + 9] = 0.8f;      // Alpha
                vertices[idx + 10] = 0.0f;     // Selectat

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
                    case MOMENT_IMPACT:        r = 1.0f; g = 0.0f; b = 1.0f; break; // Magenta
                    case MOMENT_REACTIUNE:     r = 0.8f; g = 0.2f; b = 0.8f; break; // Violet
                }

                adaugaSageata(f.valoare, f.punct_aplicare, r, g, b); 
                
                adaugaMoment(f.moment, f.punct_aplicare, r, g, b);
            }
        }
    }

    return pct_curent;
}


GLFWwindow* openGLWindow(unsigned int &shaderProgram, unsigned int &gridProgram){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(1000, 800, "Motor de fizica 2D", NULL, NULL);
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
    /*unsigned int geometryShader;
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
    glCompileShader(geometryShader);
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
    }*/

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
    //glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);                                     

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
         std::cout << "ERROR::PROGRAM::FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    //glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader); 

    glEnable(GL_BLEND);                 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int gridVertex, gridFragment;

    gridVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(gridVertex, 1, &gridVertexShaderSource, NULL);
    glCompileShader(gridVertex);

    glGetShaderiv(gridVertex, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(gridVertex, 512, NULL, infoLog);
        std::cout << "Eroare la Vertex Shader Grid:\n" << infoLog << std::endl;
    }

    gridFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(gridFragment, 1, &gridFragmentShaderSource, NULL);
    glCompileShader(gridFragment);

    glGetShaderiv(gridFragment, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(gridFragment, 512, NULL, infoLog);
        std::cout << "Eroare la Fragment Shader Grid:\n" << infoLog << std::endl;
    }

    gridProgram = glCreateProgram();
    glAttachShader(gridProgram, gridVertex);
    glAttachShader(gridProgram, gridFragment);
    glLinkProgram(gridProgram);

    glGetProgramiv(gridProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(gridProgram, 512, NULL, infoLog);
        std::cout << "Eroare la Linkare Program Grid:\n" << infoLog << std::endl;
    }

    glDeleteShader(gridVertex);
    glDeleteShader(gridFragment);

    glEnable(GL_BLEND);                 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

void initBuffers(unsigned int &VAO, unsigned int &VBO, unsigned int &gridVAO, unsigned int &gridVBO) {
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

    for (unsigned int i = 0; i <= 7; i++) {
        glVertexAttribDivisor(i, 1);
    }

    float quadVertices[] = {
        // x, y 
        -1.0f,  1.0f, 
        -1.0f, -1.0f, 
         1.0f, -1.0f, 
        -1.0f,  1.0f, 
         1.0f, -1.0f, 
         1.0f,  1.0f  
    };

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindVertexArray(0);
}

void drawGrid(editor &E) {
    if (!E.flag.arata_grid) return; // Desenam doar daca bifa din UI e activa

    glUseProgram(E.gridShaderProgram);
    glBindVertexArray(E.gridVAO);

    // Trimitem datele camerei
    glUniform2f(glGetUniformLocation(E.gridShaderProgram, "cameraOffset"), E.camera.x, E.camera.y);
    glUniform1f(glGetUniformLocation(E.gridShaderProgram, "zoom"), E.camera.zoom);
    glUniform1f(glGetUniformLocation(E.gridShaderProgram, "aspect_ratio"), E.camera.aspect_ratio);

    // Desenam dreptunghiul care acopera ecranul (6 varfuri)
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    glUniform1f(scaleVertexLoc, 1.0f / E.camera.zoom);

    int camOffsetLoc = glGetUniformLocation(shaderProgram, "cameraOffset");
    glUniform2f(camOffsetLoc, E.camera.x, E.camera.y);

    int aspectLoc = glGetUniformLocation(shaderProgram, "aspect_ratio");
    glUniform1f(aspectLoc,E.camera.aspect_ratio);

    int viewModeLoc = glGetUniformLocation(shaderProgram, "ViewMode");
    glUniform1i(viewModeLoc,E.flag.mod_vizualizare);

    //glDrawArrays(GL_POINTS, 0, totalPoints);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, totalPoints);
    glBindVertexArray(0);
}