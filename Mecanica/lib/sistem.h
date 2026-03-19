#pragma once
#include "matrice.h"
#include "legatura.h"
#include "rigid.h"
#include <cmath>
#include <vector>

class arc{

    public:

    int contorCorpA;
    int contorCorpB;
    
    float l_xA, l_yA;
    float l_xB, l_yB;

    float lungime_0;
    float k,d;  // k--constanta elastica d--constanda de dampening

    arc();

    arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0);
    // Permite definirea arcului folosind coordonate GLOBALE pentru punctele de prindere
    static arc Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d = 0.0f, float lungime_repaus = -1.0f);
    
    void aplicaFortaElastica(rigid &A, rigid &B);
};

class sistem
{
public:
    int nr_corpuri;
    int nr_legaturi;
    std::vector<rigid> corpuri;
    std::vector<legatura*> legaturi;                   // vector de pointeri
    std::vector<arc> arcuri;

    int p;                                  // p este numarul de ecuatii adaugate de legaturi (2 pt articulatii, 3 pt incastrare, etc.)
    matrice stare;                          // am sa ma refer la ecuatiile adaugate f_1,f_2... cu numele de "constrangeri"

    float k_s,k_d,g,k_a;                          //spring constant si dampening constant -- sunt encesare pentru a introduce o amortizare foarte slaba care sa anuleze erorile de tip floating-point-arithmetic
                                             // constanta gravitationala       
    matrice coeficientRestituire, coeficientFrecare;                                  
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

    void setareConstante(float spring_constant, float dampening_constant);

    void setareConstantaFrecareAer(float constanta);

    void adaugaCorpuri(rigid &r);

    void adaugaLegaturi(legatura *l);

    void adaugaArcuri(arc &a);

    void incarcaStare();

    void seteazaCoeficientRestituire(float val);

    void seteazaCoeficientFrecare(float val);

    void seteazaStare();

    void seteazaJacobian();

    void seteazaConstrangeri();

    void seteazaMatriceInertie();
};