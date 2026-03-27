#pragma once
#include "matrice.h"
#include "legatura.h"
#include "rigid.h"
#include "arc.h"
#include <cmath>
#include <vector>


enum{
MOD_RULARE = 0,
MOD_EDITARE,
MOD_PLASARE_CORP,
MOD_PLASARE_LEGATURA_1,
MOD_PLASARE_LEGATURA_2
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

class sistem
{
public:
    //int nr_corpuri;
    //int nr_legaturi;
    std::vector<rigid> corpuri;
    std::vector<legatura*> legaturi;                   // vector de pointeri
    //std::vector<std::unique_ptr<legatura>> legaturi;
    std::vector<arc> arcuri;                           // nu avem nevoie sa tinem minte numarl de arcuri, il putem scoate din arcuri.size()
    std::vector<fantomaUI> elementeUI;

    int p;                                  // p este numarul de ecuatii adaugate de legaturi (2 pt articulatii, 3 pt incastrare, etc.)
    matrice stare;                          // am sa ma refer la ecuatiile adaugate f_1,f_2... cu numele de "constrangeri"

    float k_s,k_d,g;                          //spring constant si dampening constant -- sunt encesare pentru a introduce o amortizare foarte slaba care sa anuleze erorile de tip floating-point-arithmetic
                                             // constanta gravitationala                                      
    matrice Q, J_F, A, A_inv, Lambda, JdotQ;       // Q - vectorul fortelor externe
                                            // J_f - Jacobianul legaturilor
                                            //JdotQ - produsul dintre derivata jacobianului si derivata coordonatelor
    matrice F, Fpunct;                      // sunt folosite pentru corectia erorii, impreuna cu constantele k_s si k_d
    
    int id_corp_lume;

    float mouse_x;
    float mouse_y;

    fantomaUI fantoma_plasare_corp;
    fantomaUI fantoma_legatura;

    int legatura_corp_A = -1;

    int mod_curent = 0;     //-- 0-running 1-edit 2-plasare-corp 3-plasare-legatura1 4-plasare-legatura2
    int cadru_activ = 0;

    std::vector<int> corpuriSelectate;    

    sistem();

    ~sistem();

    void setareConstantaGravitationala(float grav);

    void setareConstanteStabilizare(float spring_constant, float dampening_constant);

    void adaugaCorpuri(rigid &r);

    void adaugaLegaturi(legatura *l);

    void adaugaArcuri(arc &a);

    void incarcaStare();

    void seteazaStare();

    void seteazaJacobian();

    void seteazaConstrangeri();

    void seteazaMatriceInertie();

    void seteazaForteExterne();

    void eliminaCorp(int index);
    
    void eliminaLegatura(int index);

    void eliminaArc(int index);

    void plafonareViteze();
};