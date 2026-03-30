#include "scena.h"
#include <cmath>

const float PI = 3.1415926535f;

void incarcaScenaInitiala(sistem &S) {
    
    // 1. FUNDATIA
    // Curatam tot si ne asiguram ca elementul 0 este Lumea (fixa, masa infinita)
    S.corpuri.clear();
    S.legaturi.clear();
    S.arcuri.clear();
    S.p = 0;

    rigid lume = rigid::Fix(0.0f, 0.0f);
    S.adaugaCorpuri(lume);

    // ============================================================
    // PARAMETRII PROBLEMEI
    // ============================================================
    const float alpha = 10.0f * PI / 180.0f;
    
    const float L_OAB = 10.0f;  
    const float M_OAB = 15.0f;  

    const float L_O1B = 4.0f;   
    const float M_O1B = 20.0f;  

    const float R_disc = 0.5f;  
    const float M_disc = 3.0f;  

    const float grosime = 0.4f;

    // ============================================================
    // 2. ADAUGARE CORPURI
    // ============================================================
    
    // Bara inclinata OAB
    float cx_OAB = (L_OAB / 2.0f) * std::cos(alpha);
    float cy_OAB = (L_OAB / 2.0f) * std::sin(alpha);

    rigid bara_OAB = rigid::Bara(cx_OAB, cy_OAB, L_OAB, grosime, M_OAB);
    bara_OAB.phi = alpha;
    bara_OAB.collider.culoare = {0.8f, 0.6f, 0.2f, 1.0f};
    bara_OAB.material = materiale::Piatra;
    S.adaugaCorpuri(bara_OAB);
    int id_OAB = S.corpuri.size() - 1;

    // Punctul B unde se intalnesc barele
    float dist_B = 6.0f; 
    float B_x = dist_B * std::cos(alpha);
    float B_y = dist_B * std::sin(alpha);

    // Bara orizontala O1B
    float cx_O1B = B_x + (L_O1B / 2.0f);
    float cy_O1B = B_y - grosime;

    rigid bara_O1B = rigid::Bara(cx_O1B, cy_O1B, L_O1B, grosime, M_O1B);
    bara_O1B.phi = 0.0f;
    bara_O1B.collider.culoare = {0.4f, 0.7f, 1.0f, 1.0f};
    bara_O1B.material = materiale::Piatra;
    S.adaugaCorpuri(bara_O1B);
    int id_O1B = S.corpuri.size() - 1;

    // Discul C (asezat pe bara OAB)
    float axa_x = 8.5f * std::cos(alpha);
    float axa_y = 8.5f * std::sin(alpha);
    float normal_x = -std::sin(alpha);
    float normal_y =  std::cos(alpha);
    float distanta_ridicare = R_disc + (grosime / 2.0f) + 0.05f; 

    rigid disc = rigid::Disc(axa_x + normal_x * distanta_ridicare, 
                             axa_y + normal_y * distanta_ridicare, 
                             R_disc, M_disc);
    disc.collider.culoare = {1.0f, 0.3f, 0.3f, 1.0f};
    disc.material = materiale::Piatra;
    S.adaugaCorpuri(disc);

    // ============================================================
    // 3. LEGATURI (Aici e secretul sa nu cada)
    // ============================================================
    
    // a) Prindem bara OAB de Lume (in originea 0,0)
    S.adaugaLegaturi(articulatie::Creaza(S.corpuri[0], S.corpuri[id_OAB], 0.0f, 0.0f));

    // b) Prindem bara orizontala O1B de Lume in punctul O1 (capatul din dreapta)
    float O1_x = cx_O1B + (L_O1B / 2.0f);
    float O1_y = cy_O1B;
    S.adaugaLegaturi(incastrare::Creaza(S.corpuri[0], S.corpuri[id_O1B], O1_x, O1_y));


    // 4. Initializam matematica fizicii
    S.setareConstanteStabilizare(1000.0f, 500.0f);
    S.incarcaStare();
    S.seteazaMatriceInertie();
}