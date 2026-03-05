#pragma once
#include "sistem.h"
#include "matrice.h"
#include <algorithm> // Pentru std::swap            //AI

//rezolva sisteme olonoame scleronome, cu legaturi bilaterale

//functie facuta predominant de AI
void calculeazaMultiplicatori(sistem &S, float t){                   //rezolva sistemul (J * A^-1 * J^T) Lambda = - JpunctQpunct - J * A^-1 * Q - k_s*F - k_d*Fpunct 
                                                                     
    // 1. Construim componentele ecuatiei M * Lambda = B
    
    // A_inv se calculeaza usor, A fiind diagonala
    matrice A_inv = S.A.inverse(); 
    matrice J_T = S.J_F ^ "T";

    // Calculam M = J_F * A_inv * J_F^T (Dimensiunea va fi p x p)
    matrice M = S.J_F * A_inv * J_T;

    for(int i = 0; i < S.p; i++) {              // adaugam o componenta infinitezimal de mica pe diagonala principala, pentru nu permite matricea sa devina singulara
        M(i, i) += 1e-4f; 
    }

    // Calculam termenul liber B = - J_F * A_inv * Q - JdotQ (Dimensiunea va fi p x 1)
    matrice J_Ainv_Q = S.J_F * A_inv * S.Q;
    matrice B(S.p, 1);
    for(int i = 0; i < S.p; i++) {
        B(i, 0) = -J_Ainv_Q(i, 0) - S.JdotQ(i, 0) - S.k_s * S.F(i, 0) - S.k_d * S.Fpunct(i, 0); // Adaugam termenii de corectie Baumgarte
    }

    int p = S.p;

    // 2. Eliminarea Gaussiana cu pivotare partiala
    for (int k = 0; k < p; k++) {
        // Cautam valoarea maxima (pivotul) pe coloana curenta, de la linia k in jos
        float maxVal = std::abs(M(k, k));
        int maxRow = k;
        for (int i = k + 1; i < p; i++) {
            if (std::abs(M(i, k)) > maxVal) {
                maxVal = std::abs(M(i, k));
                maxRow = i;
            }
        }

        // Daca pivotul gasit nu e pe linia curenta, interschimbam toata linia
        if (maxRow != k) {
            for (int j = k; j < p; j++) {
                std::swap(M(k, j), M(maxRow, j));
            }
            std::swap(B(k, 0), B(maxRow, 0));
        }

        // Verificam singularitatea (daca pivotul e inca foarte aproape de zero)
        if (std::abs(M(k, k)) < 1e-6f) {
            continue; // Sarim peste pentru a nu imparti la zero
        }

        // Eliminam elementele de sub pivot pentru a forma un triunghi de zerouri
        for (int i = k + 1; i < p; i++) {
            float factor = M(i, k) / M(k, k);
            B(i, 0) = B(i, 0) - factor * B(k, 0);
            
            for (int j = k; j < p; j++) {
                M(i, j) = M(i, j) - factor * M(k, j);
            }
        }
    }

    // 3. Substitutia inapoi pentru a afla efectiv fortele Lambda
    S.Lambda = matrice(p, 1);
    for (int i = p - 1; i >= 0; i--) {
        float suma = B(i, 0);
        for (int j = i + 1; j < p; j++) {
            suma -= M(i, j) * S.Lambda(j, 0);
        }
        
        if (std::abs(M(i, i)) > 1e-6f) {
            S.Lambda(i, 0) = suma / M(i, i);
        } else {
            S.Lambda(i, 0) = 0.0f; // Siguranta in caz de singularitate irecuperabila
        }
    }

}

matrice derivata(sistem &S, const matrice &stare_curenta, float t)
{
    // Pentru a calcula derivata starii (viteze si acceleratii) la un moment dat,
    // trebuie sa recalculam fortele si constrangerile pentru starea curenta (pozitii si viteze).
    // Aceasta functie modifica temporar starea sistemului S pentru a face aceste calcule.

    // 1. Salvam starea originala a sistemului pentru a o putea restaura la final.
    matrice stare_originala = S.stare;

    // 2. Setam sistemul la starea curenta pentru care calculam derivata.
    S.stare = stare_curenta;
    S.seteazaStare(); // Actualizeaza pozitiile si vitezele corpurilor din sistem.

    // 3. Recalculam marimile care depind de stare.
    S.seteazaForteExterne();      // Recalculeaza vectorul de forte externe Q.
    S.seteazaJacobian();          // Recalculeaza Jacobianul J_F si termenul Jdot * Qdot.
    S.seteazaConstrangeri();      // Calculam erorile de constrangere F si Fpunct
    calculeazaMultiplicatori(S, t); // Calculeaza multiplicatorii Lagrange Lambda.

    // 4. Calculam acceleratiile folosind ecuatia de miscare.
    // q_ddot = A^-1 * (Q + J^T * Lambda)
    int nr = S.nr_corpuri;
    matrice acc(3 * nr, 1);
    matrice A_inv = S.A.inverse();
    matrice J_T = S.J_F ^ "T";
    acc = A_inv * (S.Q + J_T * S.Lambda);

    // 5. Construim vectorul derivatei starii: [viteze, acceleratii].
    matrice stare_derivata(6 * nr, 1);

    for(int i = 0; i < 3 * nr; i++) {
        stare_derivata(i, 0) = stare_curenta(i + 3 * nr, 0); // Viteze (pozitiile derivate)
        stare_derivata(i + 3 * nr, 0) = acc(i, 0);           // Acceleratii (vitezele derivate)
    }

    // 6. Restauram starea originala a sistemului.
    S.stare = stare_originala;
    S.seteazaStare();               // pune in coordonatele fiecarui corp valorile din vectorul stare

    return stare_derivata;
}

matrice RK4(sistem &S, float dt, float t) {

    int dim  = S.stare.linii;
    matrice k1(dim,1), k2(dim,1), k3(dim,1), k4(dim,1), stare_noua(dim,1);

    k1 = derivata(S, S.stare, t) * dt;
    k2 = derivata(S, S.stare + k1 * 0.5f, t + 0.5f * dt) * dt;
    k3 = derivata(S, S.stare + k2 * 0.5f, t + 0.5f * dt) * dt;
    k4 = derivata(S, S.stare + k3, t + dt) * dt;

    stare_noua = S.stare + (k1 + k2 * 2.0f + k3 * 2.0f + k4) * (1.0f / 6.0f);

    return stare_noua;
}
