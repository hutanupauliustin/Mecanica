#pragma once
#include "sistem.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <memory>

class InstrumentEditor;

enum ModEditor {
    MOD_RULARE = 0,
    MOD_EDITARE = 1,
};

enum TipObiect {
    TIP_CORP,
    TIP_LEGATURA,
    TIP_GENERATOR_FORTA
};

struct ObiectSelectat {
    TipObiect tip;
    int id;

    bool operator==(const ObiectSelectat& altul) const {
        return tip == altul.tip && id == altul.id;
    }
};

enum{
    POZITIE_X = 0,
    POZITIE_Y,
    POZITIE_PHI,
    VITEZA_X,
    VITEZA_Y,
    VITEZA_OMEGA,
    ACCELERATIE_X,
    ACCELERATIE_Y,
    ACCELERATIE_EPSILON,
    TOTAL_PARAMETRII
};

struct fantomaUI {
    bool activa = false;
    float x = 0.0f;
    float y = 0.0f;
    float phi = 0.0f;
    int tip; // 0 = Punct, 1 = Cerc, 2 = Dreptunghi 
    float dim1;
    float dim2;
    culoare col;
};

struct editorFlags {
    bool arata_energie = false;
    bool arata_forte = false;
    bool arata_grid = true;
    bool arata_grafic = true;
    int mod_vizualizare = 0; // standard, viteze, acceleratii
    float culoare_fundal[3] = {0.1f, 0.1f, 0.1f};
    bool salveaza_log_legaturi_la_final = false;
    bool salveaza_log_corpuri_la_final = false;
    bool inregistreaza_date = false;

};

struct IstoricCorp{
    std::vector<float> axe[TOTAL_PARAMETRII];
    std::vector<float> timpAfisat;
    int offset = 0;
    int capacitate_maxima = 7200; // 60 FPS * 120 de secunde = 2 minute

    IstoricCorp() {
        timpAfisat.reserve(capacitate_maxima);
        for(size_t i = 0; i < TOTAL_PARAMETRII; i++) {
            axe[i].reserve(capacitate_maxima);
        }
    }

};

struct IstoricLegatura {
    std::vector<float> timpAfisat;
    
    std::vector<float> fortaModul; 
    std::vector<float> fortaX;
    std::vector<float> fortaY;
    std::vector<float> moment;
    
    int offset = 0;
    int capacitate_maxima = 7200; // 60 FPS * 120 de secunde = 2 minute

    IstoricLegatura() {
        timpAfisat.reserve(capacitate_maxima);
        fortaModul.reserve(capacitate_maxima);
        fortaX.reserve(capacitate_maxima);
        fortaY.reserve(capacitate_maxima);
        moment.reserve(capacitate_maxima);
    }
};

class editor{

    public:

    editor();
    ~editor();

    ModEditor mod_curent;

    std::unique_ptr<InstrumentEditor> instrumentCurent;
    void schimbaInstrumentCurent(InstrumentEditor* instrumentNou);
    
    float mouse_x;
    float mouse_y;

    struct cameraType {
        float x = 0.0f;
        float y = 0.0f;
        float zoom = 10.0f;
        float aspect_ratio = 1.33f;

        vec2 screenToWorld(float mouseX, float mouseY, int screenWidth, int screenHeight) {
            float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
            float ndcY = 1.0f - (2.0f * mouseY) / screenHeight; 
            return vec2(ndcX * zoom * aspect_ratio + x, ndcY * zoom + y);
        }
    } camera ;

    
    std::vector<fantomaUI> elementeUI;
    std::vector<ObiectSelectat> elementeSubMouse;
    std::vector<ObiectSelectat> elementeSelectate;

    unsigned int VAO, VBO;
    std::vector<float> vertexBuffer;
    size_t total_elemente;

    unsigned int shaderProgram;
    unsigned int vertexStride = 17;
    unsigned int frameCount;

    unsigned int gridVAO, gridVBO;
    unsigned int gridShaderProgram;

    editorFlags flag;
    int cadru_activ;

    std::vector<IstoricCorp> valoriSimulate;
    std::vector<IstoricLegatura> valoriLegaturi;
    GLFWwindow* window = nullptr;

    std::string nume_fisier_export = ".temp_log_corpuri.csv";
    std::string nume_fisier_export_legaturi = ".temp_log_legaturi.csv";

    std::ofstream fisier_export;
    std::ofstream fisier_export_legaturi;

    ObiectSelectat gasesteObiectSubMouse(sistem &S);
    void mutaCorp(sistem &S, int idCorp, float offsetX, float offsetY);

    void sincronizeazaMemorie(sistem &S);
    void incarcaDatePentruGrafic(sistem &S);

    void salveazaLogCorpuri();
    void salveazaLogLegaturi();

    void updateMousePosition();
    
    void proceseazaClick(sistem &S, int buton, int actiune);
    void proceseazaMiscareMouse(sistem &S);
    
};
