#pragma once
#include "matrice.h"
#include "legatura.h"
#include "rigid.h"
#include <cmath>
#include <vector>

class sistem
{
public:
    int nr_corpuri;
    int nr_legaturi;
    std::vector<rigid> corpuri;
    std::vector<legatura*> legaturi;                   // vector de pointeri

    int p;                                  // p este numarul de ecuatii adaugate de legaturi (2 pt articulatii, 3 pt incastrare, etc.)
    matrice stare;                          // am sa ma refer la ecuatiile adaugate f_1,f_2... cu numele de "constrangeri"

    float k_s,k_d,g,k_a;                          //spring constant si dampening constant -- sunt encesare pentru a introduce o amortizare foarte slaba care sa anuleze erorile de tip floating-point-arithmetic
                                             // constanta gravitationala       
    public:                                   
    matrice Q, J_F, A, A_inv, Lambda, JdotQ;       // Q - vectorul fortelor externe
                                            // J_f - Jacobianul legaturilor
                                            //JdotQ - produsul dintre derivata jacobianului si derivata coordonatelor
    matrice F, Fpunct;                      // sunt folosite pentru corectia erorii, impreuna cu constantele k_s si k_d
    
    sistem()
    {                                       // A - matricea de inertie
        nr_corpuri = 0;
        nr_legaturi = 0;
                    // Lambda - vectorul multiplicatorilor lui Lagrange
        p = 0;
        k_d = 0.0f;
        k_s = 0.0f;
        g = 9.81f; // Initializare implicita
        k_a = 0.0f;

        stare = matrice(6 * nr_corpuri, 1);
    }

    ~sistem()
    {
        for (int i = 0; i < nr_legaturi; i++)
        {
            delete legaturi[i];
        }
    }

    void setareConstantaGravitationala(float grav){
        g = grav;
    }

    void setareConstante(float spring_constant, float dampening_constant){
        k_s = spring_constant;
        k_d = dampening_constant;
    }

    void setareConstantaFrecareAer(float constanta){
        k_a = constanta;
    }

    void adaugaCorpuri(rigid &r){
        r.index = corpuri.size();
        corpuri.push_back(r);
        nr_corpuri = corpuri.size();
    }

    void adaugaLegaturi(legatura *l)
    {
        legaturi.push_back(l);
        nr_legaturi = legaturi.size();
        p += l->getNumarEcuatii();
    }

    void incarcaStare(){
        stare = matrice(6 * nr_corpuri, 1);
        
        for (int i = 0; i < nr_corpuri; i++)
        {
            stare(i * 3, 0) = corpuri[i].x;
            stare(i * 3 + 1, 0) = corpuri[i].y;
            stare(i * 3 + 2, 0) = corpuri[i].phi;
            stare(i * 3 + 3 * nr_corpuri, 0) = corpuri[i].v_x;
            stare(i * 3 + 1 + 3 * nr_corpuri, 0) = corpuri[i].v_y;
            stare(i * 3 + 2 + 3 * nr_corpuri, 0) = corpuri[i].omega;
        }
    }

    void seteazaStare(){
        for (int i = 0; i < nr_corpuri; i++)
        {
            corpuri[i].x = stare(i * 3, 0);
            corpuri[i].y = stare(i * 3 + 1, 0);
            corpuri[i].phi = stare(i * 3 + 2, 0);
            corpuri[i].v_x = stare(i * 3 + 3 * nr_corpuri, 0);
            corpuri[i].v_y = stare(i * 3 + 1 + 3 * nr_corpuri, 0);
            corpuri[i].omega = stare(i * 3 + 2 + 3 * nr_corpuri, 0);
        }
    }

    void seteazaJacobian()
    {

        if (J_F.linii != p || J_F.coloane != 3 * nr_corpuri)
        {
            J_F = matrice('0', p, 3 * nr_corpuri);
        }
        else
        {
            for (int i = 0; i < J_F.linii; i++)
            {
                for (int j = 0; j < J_F.coloane; j++)
                {
                    J_F(i, j) = 0.0f;
                }
            }
        }                           //redeclaram jacobianul, daca trebuie modificat numarul de valori
                                    //daca nu, il facem 0 peste tot        
        if(JdotQ.linii != p || JdotQ.coloane != 1){
            JdotQ = matrice('0',p,1);
        } else {
            for(int i = 0; i < p; i++){
                JdotQ(i,0) = 0.0f;
            }
        }

        int rand_constrangere = 0;

        for (int i = 0; i < legaturi.size(); i++)
        {

            legaturi[i]->calculeazaJacobian(J_F, rand_constrangere, stare);
            legaturi[i]->calculeazaJpunctQpunct(JdotQ, rand_constrangere, stare, nr_corpuri);
            rand_constrangere += legaturi[i]->getNumarEcuatii();
        }
    }

void seteazaConstrangeri()
    {

        if (F.linii != p || F.coloane != 1)
        {
            F = matrice('0', p, 1);
        }
        else
        {
            for (int i = 0; i < F.linii; i++)
            {
                F(i, 0) = 0.0f;
            }

        }

        if (Fpunct.linii != p || Fpunct.coloane != 1)
        {
            Fpunct = matrice('0', p, 1);
        }
        else
        {
            for (int i = 0; i < Fpunct.linii; i++)
                Fpunct(i, 0) = 0.0f;
        }

        int rand_constrangere = 0;

        for (int i = 0; i < legaturi.size(); i++)
        {

            legaturi[i]->calculeazaConstrangere(F, rand_constrangere, stare);
            legaturi[i]->calculeazaConstrangereDerivate(Fpunct, rand_constrangere, stare, nr_corpuri);
            rand_constrangere += legaturi[i]->getNumarEcuatii();
        }
    }

    void seteazaMatriceInertie()
    {
        if (A.linii != 3 * nr_corpuri || A.coloane != 3 * nr_corpuri) {
            A = matrice('0', 3 * nr_corpuri, 3 * nr_corpuri);
        } else {
            for(int i =0; i < 3*nr_corpuri; i++)
                for(int j = 0; j < 3*nr_corpuri; j++){
                    A(i,j) = 0;
                }
        }

        for (int i = 0; i < nr_corpuri; i++)
        {
            A(3 * i,     3 * i)     = corpuri[i].M; 
            A(3 * i + 1, 3 * i + 1) = corpuri[i].M; 
            A(3 * i + 2, 3 * i + 2) = corpuri[i].J; 
        }   
        
        A_inv = A.inverse();
    }

    void seteazaForteExterne()
    {
        if (Q.linii != 3 * nr_corpuri || Q.coloane != 1) {
            Q = matrice(3 * nr_corpuri, 1);
        }

        for (int i = 0; i < nr_corpuri; i++)
        {
            corpuri[i].aflaForteProprii(g, k_a); // Trimitem g-ul sistemului catre corp
            Q(3 * i, 0) = corpuri[i].f_x;
            Q(3 * i + 1, 0) = corpuri[i].f_y;
            Q(3 * i + 2, 0) = corpuri[i].moment;
        }
    }
};