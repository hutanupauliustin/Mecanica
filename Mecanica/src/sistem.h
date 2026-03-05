#pragma once
#include "matrice.h"
#include "legatura.h"
#include "rigid.h"
#include <cmath>

class sistem
{
public:
    int nr_corpuri;
    int nr_legaturi;
    rigid *corpuri;
    legatura **legaturi;                    // vector de pointeri
    int legaturi_adaugate;
    int corpuri_adaugate;
    int p;                                  // p este numarul de ecuatii adaugate de legaturi (2 pt articulatii, 3 pt incastrare, etc.)
    matrice stare;                          // am sa ma refer la ecuatiile adaugate f_1,f_2... cu numele de "constrangeri"

    matrice Q, J_F, A, Lambda, JdotQ;       // Q - vectorul fortelor externe
                                            // J_f - Jacobianul legaturilor
                                            //JdotQ - produsul dintre derivata jacobianului si derivata coordonatelor
    sistem(int nr_corpuri, int nr_legaturi)
    {                                       // A - matricea de inertie
        this->nr_corpuri = nr_corpuri;                     // Lambda - vectorul multiplicatorilor lui Lagrange
        this->nr_legaturi = nr_legaturi;
        legaturi_adaugate = 0;
        corpuri_adaugate = 0;
        p = 0;

        corpuri = new rigid[nr_corpuri];
        legaturi = new legatura *[nr_legaturi];
        stare = matrice(6 * nr_corpuri, 1);
    }

    ~sistem()
    {
        delete[] corpuri;
        for (int i = 0; i < nr_legaturi; i++)
        {
            delete legaturi[i];
        }
        delete[] legaturi;
    }

    void adaugaCorpuri(rigid &r){
        if (corpuri_adaugate < nr_corpuri)
        {
            corpuri[corpuri_adaugate] = r;
            corpuri_adaugate++;
        }
    }

    void adaugaLegaturi(legatura *l)
    {
        if (legaturi_adaugate < nr_legaturi)
        {
            legaturi[legaturi_adaugate] = l;
            legaturi_adaugate++;

            p += l->getNumarEcuatii();
        }
    }

    void incarcaStare(){
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

        for (int i = 0; i < legaturi_adaugate; i++)
        {

            legaturi[i]->calculeazaJacobian(J_F, rand_constrangere, stare);
            legaturi[i]->calculeazaJpunctQpunct(JdotQ, rand_constrangere, stare, nr_corpuri);
            rand_constrangere += legaturi[i]->getNumarEcuatii();
        }
    }

    void seteazaMatriceInertie()
    {
        if (A.linii != 3 * nr_corpuri || A.coloane != 3 * nr_corpuri) {
            A = matrice('0', 3 * nr_corpuri, 3 * nr_corpuri);
        } else {
            for(int i =0; i < 3*nr_corpuri; i++)
                for(int j = 0; j < 3*nr_corpuri; j++)
                    A(i,j) = 0;
        }

        for (int i = 0; i < nr_corpuri; i++)
        {
            A(3 * i,     3 * i)     = corpuri[i].M; 
            A(3 * i + 1, 3 * i + 1) = corpuri[i].M; 
            A(3 * i + 2, 3 * i + 2) = corpuri[i].J; 
        }    
    }

    void seteazaForteExterne()
    {
        if (Q.linii != 3 * nr_corpuri || Q.coloane != 1) {
            Q = matrice(3 * nr_corpuri, 1);
        }

        for (int i = 0; i < nr_corpuri; i++)
        {
            corpuri[i].aflaForteProprii();
            Q(3 * i, 0) = corpuri[i].f_x;
            Q(3 * i + 1, 0) = corpuri[i].f_y;
            Q(3 * i + 2, 0) = corpuri[i].moment;
        }
    }
};