#include "fizica.h"

matrice RK4(const matrice &x, float dt, float t){

    int n = x.linii / 2;

    matrice k1(2*n,1), k2(2*n,1), k3(2*n,1), k4(2*n,1), x_nou(2*n,1);

    k1 = f(x, t) * dt; 
    k2 = f(x + k1 * 0.5f, t + 0.5f*dt) * dt;
    k3 = f(x + k2 * 0.5f, t + 0.5f*dt) * dt;
    k4 = f(x + k3, t + dt) * dt;

    x_nou = x + (k1 + k2 * 2.0f + k3 * 2.0f + k4) * (1.0f / 6.0f);

    return x_nou;
}

matrice f(const matrice &x, float t) {
   
    return matrice();
}

void seteazaForte(sistem &S, float t){
    for(int i = 0; i < S.n; i++){
        
        S.v[i].greutateProprie();

        for(int j = 0; j < S.n; j++){   //fortele coulombiene
            if(i == j)
                continue;
            float d = S.distanta(S.v[i],S.v[j]);
            if(d < 0.001f) 
                d = 0.001f;
            float modul = (8.99e9 * S.v[i].sarcina * S.v[j].sarcina ) / (d*d*d);
            matrice r(2,1);
            r = S.vectorPozitie(S.v[i],S.v[j]);
            S.v[i].fx += modul * r(0,0);
            S.v[i].fy += modul * r(1,0);
        }
    }
}