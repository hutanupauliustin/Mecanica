#include "arc.h"

arc::arc(){
        contorCorpA = 0;
        contorCorpB = 0;
        lungime_0 = 0.0f;
        k = 0.0f;
        d = 0.0f;

    }

    arc::arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0)
        : contorCorpA(a), contorCorpB(b), l_A(lxa,lya), l_B(lxb,lyb), k(k_val), d(d_val), lungime_0(l0) {}

    // Permite definirea arcului folosind coordonate GLOBALE pentru punctele de prindere
    arc arc::Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d, float lungime_repaus) {
        // Calculam vectorul de la centrul corpului la punctul de legatura (in coordonate globale)

        vec2 globalA (globalXA, globalYA);
        vec2 globalB (globalXB, globalYB);

        vec2 localA = A.globalToLocal(globalA);
        vec2 localB = B.globalToLocal(globalB);

        // Daca nu se specifica o lungime de repaus (valoare negativa), o calculam ca distanta curenta dintre puncte
        float l0 = lungime_repaus;
        if (l0 < 0.0f) {
            l0 = std::sqrt((globalXB - globalXA) * (globalXB - globalXA) + (globalYB - globalYA) * (globalYB - globalYA));
        }

        return arc(A.index, B.index, localA.x, localA.y, localB.x, localB.y, constanta_k, constanta_d, l0);
    }
    

    void arc::aplicaFortaElastica(rigid &A, rigid &B){        
        
        vec2 poz1, poz2;
        vec2 viteza1, viteza2;

        poz1 = A.localToGlobal(l_A);
        poz2 = B.localToGlobal(l_B);

        viteza1 = A.vitezaAbsolutaPunct(l_A);
        viteza2 = B.vitezaAbsolutaPunct(l_B);

        float l = (poz1 - poz2).modul();

        vec2 directie;

        if (l > 0.0001f) {
            directie = (poz2 - poz1) / l;
        } else {
            l = 0.0001f; // Prevenim impartirea la 0 cand corpurile sunt suprapuse perfect (previne NaN Crash)
        }

        float viteza_rel = (viteza2 - viteza1).scalar(directie);
        float valoare_forta = -k * (l - lungime_0) - d * viteza_rel;      

        A.adauagaForte(-valoare_forta,poz1.x, poz1.y,directie.x,directie.y);
        B.adauagaForte( valoare_forta,poz2.x, poz2.y,directie.x,directie.y);


    }
    