#include "scena.h"
#include <cmath>

const float PI = 3.1415926535f;

void incarcaScenaInitiala(sistem &S) {
    // ============================================================
    // PARAMETRII PROBLEMEI
    // ============================================================
    const float alpha = 33.0f * PI / 180.0f;
    
    const float L_OAB = 10.0f;  
    const float M_OAB = 15.0f;  

    const float L_O1B = 4.0f;   
    const float M_O1B = 20.0f;  

    const float R_disc = 0.5f;  
    const float M_disc = 3.0f;  

    const float grosime_OAB = 0.4f;
    const float grosime_O1B = 0.4f;

    // 1. Bara inclinata OAB
    float cx_OAB = (L_OAB / 2.0f) * std::cos(alpha);
    float cy_OAB = (L_OAB / 2.0f) * std::sin(alpha);

    rigid bara_OAB = rigid::Bara(cx_OAB, cy_OAB, L_OAB, grosime_OAB, M_OAB);
    bara_OAB.phi = alpha;
    bara_OAB.collider.culoare = {0.8f, 0.6f, 0.2f, 1.0f};
    bara_OAB.material = materiale::Lemn;
    S.adaugaCorpuri(bara_OAB);
    int id_OAB = S.corpuri.size() - 1;

    // 2. Bara orizontala O1B
    float dist_B = 6.0f; 
    float B_x = dist_B * std::cos(alpha);
    float B_y = dist_B * std::sin(alpha);

    float offset_y_OAB = (grosime_OAB / 2.0f) / std::cos(alpha);
    float offset_y_O1B = grosime_O1B / 2.0f;
    float coborare_totala = offset_y_OAB + offset_y_O1B + 0.05f; 

    float cx_O1B = B_x + (L_O1B / 2.0f);
    float cy_O1B = B_y - coborare_totala;

    rigid bara_O1B = rigid::Bara(cx_O1B, cy_O1B, L_O1B, grosime_O1B, M_O1B);
    bara_O1B.phi = 0.0f;
    bara_O1B.collider.culoare = {0.4f, 0.7f, 1.0f, 1.0f};
    bara_O1B.material = materiale::Lemn;
    S.adaugaCorpuri(bara_O1B);
    int id_O1B = S.corpuri.size() - 1;

    // 3. Discul C 
    float dist_C = 8.5f; 
    
    float axa_x = dist_C * std::cos(alpha);
    float axa_y = dist_C * std::sin(alpha);

    float normal_x = -std::sin(alpha);
    float normal_y =  std::cos(alpha);
    
    float distanta_ridicare = R_disc + (grosime_OAB / 2.0f) + 0.05f; 

    rigid disc = rigid::Disc(axa_x + normal_x * distanta_ridicare, 
                             axa_y + normal_y * distanta_ridicare, 
                             R_disc, M_disc);
    disc.collider.culoare = {1.0f, 0.3f, 0.3f, 1.0f};
    disc.material = materiale::Cauciuc; 
    S.adaugaCorpuri(disc);

    // 4. Legaturi 
    float O1_x = cx_O1B + (L_O1B / 2.0f);
    float O1_y = cy_O1B;

    S.adaugaLegaturi(articulatie::Creaza(S.corpuri[0], S.corpuri[id_OAB], 0.0f, 0.0f));
    S.adaugaLegaturi(incastrare::Creaza(S.corpuri[0], S.corpuri[id_O1B], O1_x, O1_y));

    // 5. Pregatirea matricelor initiale
    S.setareConstanteStabilizare(10000.0f, 500.0f);
    S.incarcaStare();
    S.seteazaMatriceInertie();
}