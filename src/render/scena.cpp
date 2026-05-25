#include "scena.h"
#include <cmath>
#include "json.hpp"
#include <fstream>
#include <iostream>
#include "sistem.h"
using json = nlohmann::json;

const float PI = 3.1415926535f;

using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(tipMaterial, restituire, frecareStatica, frecareDinamica, frecareRostogolireStatica , frecareRostogolireDinamica)


void incarcaScenaInitiala(sistem &S) {

    
    // 1. FUNDATIA
    // Curatam tot si ne asiguram ca elementul 0 este Lumea (fixa, masa infinita)
    S.corpuri.clear();
    for (auto l : S.legaturi) delete l;
    S.legaturi.clear();
    for (auto f : S.surseForte) delete f;
    S.surseForte.clear();
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
    S.actualizeazaMatriceFizica();
}

void salveazaScenaJSON(sistem &S, const std::string& nume_fisier){

    json scena_json;
    scena_json["corpuri"] = json::array();

    for (size_t i = 0; i < S.corpuri.size(); i++) {
        rigid &r = S.corpuri[i];
        
        if (!r.activ || r.collider.obiectVirtual) continue; 
        
        if (r.M > 1e10f) continue; 

        json corp_json;
        corp_json["tip"] = r.collider.tip; // 1 pt Cerc 2 pt Dreptunghi
        corp_json["x"] = r.pozitie.x;
        corp_json["y"] = r.pozitie.y;
        corp_json["phi"] = r.phi;
        corp_json["dimensiune1"] = r.collider.dimensiune1;
        corp_json["dimensiune2"] = r.collider.dimensiune2;
        corp_json["masa"] = r.M;
        corp_json["material"] = r.material;
        
        corp_json["culoare"] = { r.collider.culoare.r, r.collider.culoare.g, r.collider.culoare.b, r.collider.culoare.a };

        scena_json["corpuri"].push_back(corp_json);
    }

    scena_json["legaturi"] = json::array();

    for(size_t i = 0; i < S.legaturi.size();i++){
        legatura *l = S.legaturi[i];

        if(!l || !l->activ) continue;

        json leg_json;

        leg_json["contorCorpA"] = l->contorCorpA;
        leg_json["contorCorpB"] = l->contorCorpB;


        if (articulatie* art = dynamic_cast<articulatie*>(l)) {
            leg_json["tip"] = "articulatie";
            leg_json["l_A_x"] = art->l_A.x;
            leg_json["l_A_y"] = art->l_A.y;
            leg_json["l_B_x"] = art->l_B.x;
            leg_json["l_B_y"] = art->l_B.y;
            
        }
        else if (incastrare* inc = dynamic_cast<incastrare*>(l)) {
            leg_json["tip"] = "incastrare";
            leg_json["l_A_x"] = inc->l_A.x;
            leg_json["l_A_y"] = inc->l_A.y;
            leg_json["l_B_x"] = inc->l_B.x;
            leg_json["l_B_y"] = inc->l_B.y;
            leg_json["phi_0"] = inc->phi_0;
            
        }else if (fir* f = dynamic_cast<fir*>(l)) {
            leg_json["tip"] = "fir";
            leg_json["l_A_x"] = f->l_A.x;
            leg_json["l_A_y"] = f->l_A.y;
            leg_json["l_B_x"] = f->l_B.x;
            leg_json["l_B_y"] = f->l_B.y;
            leg_json["lungime"] = f->lungime; 
        }

        scena_json["legaturi"].push_back(leg_json);
    }

    std::ofstream fisier(nume_fisier);
    if (fisier.is_open()) {
        fisier << scena_json.dump(4);
        fisier.close();
        std::cout << "Scena salvata cu succes in " << nume_fisier << "!\n";
    } else {
        std::cerr << "Eroare la deschiderea fisierului pentru salvare!\n";
    }

}

void citesteScenaJSON(sistem &S, const std::string& nume_fisier){
    std::ifstream fisier(nume_fisier);
    if( !fisier.is_open()){
        std::cerr << "Eroare: Nu am putut gasi sau deschide fisierul " << nume_fisier << "!\n";
        return;
    }

    json scena_json;
    fisier >> scena_json;
    fisier.close();

    S.corpuri.clear();
    for (auto l : S.legaturi) delete l;
    S.legaturi.clear();
    for (auto f : S.surseForte) delete f;
    S.surseForte.clear();
    S.p = 0;

    rigid lume = rigid::Fix(0.0f, 0.0f);
    S.adaugaCorpuri(lume);

    if(scena_json.contains("corpuri")){
        for(const auto& corp_json : scena_json["corpuri"]){

            int tip = corp_json["tip"];
            float x = corp_json["x"];
            float y = corp_json["y"];
            float phi = corp_json["phi"];
            float dim1 = corp_json["dimensiune1"];
            float dim2 = corp_json["dimensiune2"];
            float masa = corp_json["masa"];
            
            rigid corp_nou;

            if(tip == 1){
                corp_nou = rigid::Disc(x,y,dim1,masa);
            } else if (tip == 2){
                corp_nou = rigid::Bara(x,y,dim1,dim2,masa);
            }

            corp_nou.material = corp_json["material"];
            corp_nou.phi = phi;
            corp_nou.collider.culoare = {
                corp_json["culoare"][0],
                corp_json["culoare"][1],
                corp_json["culoare"][2],
                corp_json["culoare"][3]
            };

            S.adaugaCorpuri(corp_nou);
        }

        if(scena_json.contains("legaturi")){
        for(const auto& leg_json : scena_json["legaturi"]){
            
            // Extragem datele comune
            int idA = leg_json["contorCorpA"];
            int idB = leg_json["contorCorpB"];
            float lAx = leg_json["l_A_x"];
            float lAy = leg_json["l_A_y"];
            float lBx = leg_json["l_B_x"];
            float lBy = leg_json["l_B_y"];
            std::string tip = leg_json["tip"];

            legatura* leg_noua = nullptr;

            if (tip == "articulatie") {
                leg_noua = new articulatie(idA, idB, lAx, lAy, lBx, lBy);
            } 
            else if (tip == "incastrare") {
                float phi_0 = leg_json["phi_0"].get<float>();
                leg_noua = new incastrare(idA, idB, lAx, lAy, lBx, lBy, phi_0);
            } 
            else if (tip == "fir") {
                float lungime = leg_json["lungime"].get<float>();
                leg_noua = new fir(idA, idB, lAx, lAy, lBx, lBy, lungime);
            }

            if (leg_noua) {
                leg_noua->activ = true;
                S.adaugaLegaturi(leg_noua); 
            }
        }
    }

        S.setareConstanteStabilizare(1000.0f, 500.0f);
        S.actualizeazaMatriceFizica();

        std::cout << "Scena a fost incarcata cu succes din " << nume_fisier << "!\n";
    }
}