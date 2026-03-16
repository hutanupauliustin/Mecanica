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

    arc(){
        contorCorpA = 0;
        contorCorpB = 0;
        l_xA = 0.0f;
        l_yA = 0.0f;
        l_xB = 0.0f;
        l_yB = 0.0f;
        lungime_0 = 0.0f;
        k = 0.0f;
        d = 0.0f;

    }

    arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0)
        : contorCorpA(a), contorCorpB(b), l_xA(lxa), l_yA(lya), l_xB(lxb), l_yB(lyb), k(k_val), d(d_val), lungime_0(l0) {}

    // Permite definirea arcului folosind coordonate GLOBALE pentru punctele de prindere
    static arc Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d = 0.0f, float lungime_repaus = -1.0f) {
        // Calculam vectorul de la centrul corpului la punctul de legatura (in coordonate globale)
        float dxA = globalXA - A.x;
        float dyA = globalYA - A.y;
        
        float dxB = globalXB - B.x;
        float dyB = globalYB - B.y;

        // Transformam coordonatele in sistemul de referinta local al fiecarui corp
        float l_xA = dxA * cos(A.phi) + dyA * sin(A.phi);
        float l_yA = -dxA * sin(A.phi) + dyA * cos(A.phi);

        float l_xB = dxB * cos(B.phi) + dyB * sin(B.phi);
        float l_yB = -dxB * sin(B.phi) + dyB * cos(B.phi);

        // Daca nu se specifica o lungime de repaus (valoare negativa), o calculam ca distanta curenta dintre puncte
        float l0 = lungime_repaus;
        if (l0 < 0.0f) {
            l0 = std::sqrt((globalXB - globalXA) * (globalXB - globalXA) + (globalYB - globalYA) * (globalYB - globalYA));
        }

        return arc(A.index, B.index, l_xA, l_yA, l_xB, l_yB, constanta_k, constanta_d, l0);
    }
    

    void aplicaFortaElastica(rigid &A, rigid &B){        
        
        float x1,y1,x2,y2;
        float v_x1,v_y1,v_x2,v_y2;


        A.coordPunctPeCorp(x1,y1,l_xA, l_yA);
        B.coordPunctPeCorp(x2,y2,l_xB, l_yB);

        A.vitezaPunctPeCorp(v_x1,v_y1,l_xA, l_yA);
        B.vitezaPunctPeCorp(v_x2,v_y2,l_xB, l_yB);

        float l = std::sqrt( (x2-x1) * (x2-x1) + (y2-y1) * (y2-y1));

        float directie_x = (x2-x1) / l;               
        float directie_y = (y2-y1) / l;

        float viteza_rel = (v_x2 - v_x1) * directie_x + (v_y2 - v_y1) * directie_y;

        float fe_x = (-k * (l - lungime_0) - d * viteza_rel )* directie_x;
        float fe_y = (-k * (l - lungime_0) - d * viteza_rel )* directie_y;
        
        float r_xA = x1 - A.x;
        float r_yA = y1 - A.y;
        float r_xB = x2 - B.x;
        float r_yB = y2 - B.y;


        A.f_x += -fe_x;
        A.f_y += -fe_y;
        A.moment += -r_xA * fe_y + r_yA * fe_x;

        B.f_x += fe_x;
        B.f_y += fe_y;
        B.moment += r_xB * fe_y - r_yB * fe_x;

    }
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

    void adaugaArcuri(arc &a)
    {
        arcuri.push_back(a);
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

    void seteazaCoeficientRestituire(float val){             //valoarea din  punctul (i,j) este coeficientul dintre corpurile i si j;
        coeficientRestituire = matrice( nr_corpuri,  nr_corpuri);
        for(int i = 0; i < nr_corpuri; i++){
            for(int j = 0; j < i; j++){
                coeficientRestituire(i,j) = i == j ?  0.0f : val;
                coeficientRestituire(j,i) = val;
            }
        }

    }

    void seteazaCoeficientFrecare(float val){             //valoarea din  punctul (i,j) este coeficientul dintre corpurile i si j;
        coeficientFrecare = matrice( nr_corpuri,  nr_corpuri);
        for(int i = 0; i < nr_corpuri; i++){
            for(int j = 0; j < i; j++){
                coeficientFrecare(i,j) = i == j ?  0.0f : val;
                coeficientFrecare(j,i) = val;
            }
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

        // 1. Initializam fortele (gravitatie, frecare aer)
        for (int i = 0; i < nr_corpuri; i++) {
            corpuri[i].aflaForteProprii(g, k_a); 
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
};