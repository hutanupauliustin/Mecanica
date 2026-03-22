#include "arc.h"

arc::arc(){
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

    arc::arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0)
        : contorCorpA(a), contorCorpB(b), l_xA(lxa), l_yA(lya), l_xB(lxb), l_yB(lyb), k(k_val), d(d_val), lungime_0(l0) {}

    // Permite definirea arcului folosind coordonate GLOBALE pentru punctele de prindere
    arc arc::Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d, float lungime_repaus) {
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
    

    void arc::aplicaFortaElastica(rigid &A, rigid &B){        
        
        float x1,y1,x2,y2;
        float v_x1,v_y1,v_x2,v_y2;


        A.coordPunctPeCorp(x1,y1,l_xA, l_yA);
        B.coordPunctPeCorp(x2,y2,l_xB, l_yB);

        A.vitezaPunctPeCorp(v_x1,v_y1,l_xA, l_yA);
        B.vitezaPunctPeCorp(v_x2,v_y2,l_xB, l_yB);

        float l = std::sqrt( (x2-x1) * (x2-x1) + (y2-y1) * (y2-y1));

        float directie_x = 0.0f;
        float directie_y = 0.0f;
        if (l > 0.0001f) {
            directie_x = (x2-x1) / l;               
            directie_y = (y2-y1) / l;
        } else {
            l = 0.0001f; // Prevenim impartirea la 0 cand corpurile sunt suprapuse perfect (previne NaN Crash)
        }

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
    