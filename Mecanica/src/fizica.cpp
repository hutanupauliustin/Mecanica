#include "sistem.h"
#include "matrice.h"
#include "colliziune.h"
#include "fizica.h"

//rezolva sisteme olonoame scleronome, cu legaturi bilaterale

void calculeazaMultiplicatori(sistem &S, float t){                   //rezolva sistemul (J * A^-1 * J^T) Lambda = - JpunctQpunct - J * A^-1 * Q - k_s*F - k_d*Fpunct 
                         
    S.Lambda = matrice(S.p, 1);

    matrice M(S.p,S.p);
    matrice L(S.p,S.p);

    M =  S.J_F * S.A_inv * (S.J_F ^ "T");                   // matricea din partea stanga a sistemului

    for (int i = 0; i < S.p; i++)                           // adaugam o valoare nesemenificativa pe diagolana matricei, pentru a ne asigura ca este pozitiv definita, nu semidefinita
        M(i,i) += 1e-7f;

    for(int i = 0; i < S.p; i++){                           // calculam descompunerea Cholesky
        for(int j = 0; j <= i; j++){ 
            float suma = 0.0f;
            for(int k = 0; k < j; k++){
                suma += L(i, k) * L(j, k);
            }

            if(i == j){
                float val = M(i, i) - suma;
                L(i, i) = (val > 0.0f) ? std::sqrt(val) : 1e-6f;        // daca valoarea este ngativa, vom pune o valoare infinitezimala, nu zero, pentru a evita impartirea la 0
            } else {
                L(i, j) = (M(i, j) - suma) / L(j, j); 
            }
        }
    }

    matrice y(S.p,1);
    matrice B(S.p,1);

    B = - S.J_F * S.A_inv * S.Q - S.JdotQ - S.k_d * S.Fpunct - S.k_s * S.F;   //matricea din partea dreapta a sistemului

    for(int i = 0; i < S.p; i++){                   //calculeaza prima parte a sistemului L * ( L_T * Lambda) =  B , notand L_T * Lambda cu y
        float suma = 0.0f;
        for(int j = 0; j < i; j++){
            suma += L(i,j)* y(j,0);
        }
        y(i,0) = (B(i,0) - suma) / L(i,i);
        }

    for(int i = S.p - 1; i >= 0; i--){
        float suma = 0.0f;
        for(int j = i + 1 ; j < S.p; j++){
            suma += L(j,i)* S.Lambda(j,0);          // L(j,i) este L^T(i,j)
        } 
        S.Lambda(i,0) = (y(i,0) - suma) / L(i,i);
        }
}
    


matrice derivata(sistem &S, const matrice &stare_curenta, float t)      //facuta de AI
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


float calculeazaEnergiaTotala(sistem &S, float g) {
    float energie = 0.0f;
    
    // Parcurgem toate corpurile (presupunand ca le ai intr-un vector S.corpuri)
    for (int i = 0; i < S.nr_corpuri; i++) {
        rigid& c = S.corpuri[i];
        
        // Ignoram peretii si corpurile statice (masa infinita)
        if (c.M > 1e10f) continue; 

        float viteza_la_patrat = (c.v_x * c.v_x) + (c.v_y * c.v_y);
        
        float e_cinetica = 0.5f * c.M * viteza_la_patrat;
        float e_rotatie = 0.5f * c.J * (c.omega * c.omega);
        float e_potentiala = c.M * g * (c.y + 10); // 'g' trebuie sa fie o valoare pozitiva aici (ex: 9.81)

        energie += (e_cinetica + e_rotatie + e_potentiala);
    }
    
    return energie;
}
