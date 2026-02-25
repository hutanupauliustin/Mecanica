#include "fizica.h"

matrice RK4(sistem &S,const matrice &x, float dt, float t){

    int dim  = x.linii;

    matrice k1(dim,1), k2(dim,1), k3(dim,1), k4(dim,1), x_nou(dim,1);

    k1 = derivata(S,x,t) * dt; 
    k2 = derivata(S,x + k1 * 0.5f, t + 0.5f*dt) * dt;
    k3 = derivata(S,x + k2 * 0.5f, t + 0.5f*dt) * dt;
    k4 = derivata(S,x + k3, t + dt) * dt;

    x_nou = x + (k1 + k2 * 2.0f + k3 * 2.0f + k4) * (1.0f / 6.0f);

    return x_nou;
}

void seteazaForte(sistem &S, float t){
    for(int i = 0; i < S.n; i++){
        S.v[i].fx = 0;
        S.v[i].fy = 0;

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

 matrice derivata(sistem &S,const matrice &stare_curenta, float t){
        seteazaForte(S,t);

        S.stare = stare_curenta;
        S.seteazaStare();

        seteazaForte(S,t);

        matrice dq(4*S.n,1);

        for(int i = 0; i < S.n; i++){
            dq(2*i, 0) = S.v[i].vx;
            dq(2*i + 1, 0) = S.v[i].vy;

            float m = (S.v[i].masa > 0.0f) ? S.v[i].masa : 1.0f;

            dq(2*(i+S.n), 0) = S.v[i].fx / m;
            dq(2*(i+S.n)+ 1, 0) = S.v[i].fy / m;
        }

        return dq;
}

