#include "sistem.h"
#include <cmath>
#include <vector>

    sistem::sistem()
    {                                       // A - matricea de inertie
                    // Lambda - vectorul multiplicatorilor lui Lagrange
        p = 0;
        k_d = 0.0f;
        k_s = 0.0f;
        g = 9.81f; // Initializare implicita

        stare = matrice(2, 1);

        rigid lume = rigid::Fix(0.0f, 0.0f);
        adaugaCorpuri(lume);
        id_corp_lume = 0;

        rigid mouse = rigid::Bara(0.0f, 0.0f, 0.05f, 0.05f, 1e12f);
        mouse.collider.obiectVirtual = true;
        adaugaCorpuri(mouse);
        id_corp_mouse = 1;

    }

    sistem::~sistem()
    {
        for (int i = 0; i < this->legaturi.size(); i++)
        {
            delete legaturi[i];
        }
    }

    void sistem::setareConstantaGravitationala(float grav){
        g = grav;
    }

    void sistem::setareConstanteStabilizare(float spring_constant, float dampening_constant){
        k_s = spring_constant;
        k_d = dampening_constant;
    }


    void sistem::adaugaCorpuri(rigid &r){
        r.index = corpuri.size();
        corpuri.push_back(r);
    }

    void sistem::adaugaLegaturi(legatura *l)
    {
        legaturi.push_back(l);
        p += l->getNumarEcuatii();
    }

    void sistem::adaugaArcuri(arc &a)
    {
        arcuri.push_back(a);
    }

    void sistem::incarcaStare(){

        int nr_corpuri = this->corpuri.size();
        int nr_legaturi = this->legaturi.size();

        stare = matrice(6 * nr_corpuri, 1);
        
        for (int i = 0; i < nr_corpuri; i++)
        {
            if(this->corpuri[i].activ == 0){
                stare(i * 3, 0) = 0;
                stare(i * 3 + 1, 0) = 0;
                stare(i * 3 + 2, 0) = 0;
                stare(i * 3 + 3 * nr_corpuri, 0) = 0;
                stare(i * 3 + 1 + 3 * nr_corpuri, 0) = 0;
                stare(i * 3 + 2 + 3 * nr_corpuri, 0) = 0;
            }else{
                stare(i * 3, 0) = corpuri[i].x;
                stare(i * 3 + 1, 0) = corpuri[i].y;
                stare(i * 3 + 2, 0) = corpuri[i].phi;
                stare(i * 3 + 3 * nr_corpuri, 0) = corpuri[i].v_x;
                stare(i * 3 + 1 + 3 * nr_corpuri, 0) = corpuri[i].v_y;
                stare(i * 3 + 2 + 3 * nr_corpuri, 0) = corpuri[i].omega;
            }
        }
    }

    void sistem::seteazaStare(){

        int nr_corpuri = this->corpuri.size();

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

    void sistem::seteazaJacobian()
    {

        int nr_corpuri = this->corpuri.size();

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

void sistem::seteazaConstrangeri()
    {
        int nr_corpuri = this->corpuri.size();

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

    void sistem::seteazaMatriceInertie()
    {
        int nr_corpuri = this->corpuri.size();

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
            if(this->corpuri[i].activ == 0){
            A(3 * i,     3 * i)     = 0; 
            A(3 * i + 1, 3 * i + 1) = 0; 
            A(3 * i + 2, 3 * i + 2) = 0; 
            }else{
            A(3 * i,     3 * i)     = corpuri[i].M; 
            A(3 * i + 1, 3 * i + 1) = corpuri[i].M; 
            A(3 * i + 2, 3 * i + 2) = corpuri[i].J; 
            }
        }   
        
        A_inv = A.inverse();
    }

    void sistem::seteazaForteExterne()
    {
        int nr_corpuri = this->corpuri.size();

        if (Q.linii != 3 * nr_corpuri || Q.coloane != 1) {
            Q = matrice(3 * nr_corpuri, 1);
        }

        // 1. Initializam fortele (gravitatie, frecare aer)
        for (int i = 0; i < nr_corpuri; i++) {
            corpuri[i].aflaForteProprii(g); 
        }

        // 2. Adaugam fortele elastice 
        for (int i = 0; i < arcuri.size(); i++) {
            arcuri[i].aplicaFortaElastica(this->corpuri[arcuri[i].contorCorpA], this->corpuri[arcuri[i].contorCorpB]);
        }

        // 3. Incarcam totul in matricea sistemului
        for (int i = 0; i < nr_corpuri; i++) {
            Q(3 * i, 0) = corpuri[i].f_x;
            Q(3 * i + 1, 0) = corpuri[i].f_y;
            Q(3 * i + 2, 0) = corpuri[i].moment;
        }
    }