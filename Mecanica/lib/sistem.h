#pragma once
#include "matrice.h"
#include "legatura.h"
#include "rigid.h"
#include "arc.h"
#include <cmath>
#include <vector>

class sistem
{
public:
    int nr_corpuri;
    int nr_legaturi;
    std::vector<rigid> corpuri;
    std::vector<legatura*> legaturi;                   // vector de pointeri
    std::vector<arc> arcuri;                           // nu avem nevoie sa tinem minte numarl de arcuri, il putem scoate din arcuri.size()

    int p;                                  // p este numarul de ecuatii adaugate de legaturi (2 pt articulatii, 3 pt incastrare, etc.)
    matrice stare;                          // am sa ma refer la ecuatiile adaugate f_1,f_2... cu numele de "constrangeri"

    float k_s,k_d,g;                          //spring constant si dampening constant -- sunt encesare pentru a introduce o amortizare foarte slaba care sa anuleze erorile de tip floating-point-arithmetic
                                             // constanta gravitationala                                      
    matrice Q, J_F, A, A_inv, Lambda, JdotQ;       // Q - vectorul fortelor externe
                                            // J_f - Jacobianul legaturilor
                                            //JdotQ - produsul dintre derivata jacobianului si derivata coordonatelor
    matrice F, Fpunct;                      // sunt folosite pentru corectia erorii, impreuna cu constantele k_s si k_d
    
    int id_corp_lume;
    int id_corp_mouse;

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
};