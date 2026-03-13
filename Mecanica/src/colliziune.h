#pragma once
#include "sistem.h"
#include "matrice.h"
#include <cmath>

bool intersectareScaraLarga(sistem &S,int corpA, int corpB){        //verifica doar daca cutiile in care sunt bagate corpurile se intersecteaza
                                                                    //doar daca cutiile corpurile se supran exista posibilitatea ca ele sa se intersecteze cu adevarat, caz in care facem o verificare mai exacta
    float dist_x = std::abs(S.corpuri[corpA].x - S.corpuri[corpB].x);//costul computational de a face doua verificari in cazul in care se supranul, este mult justificat de timpul castigat prin calculul a multor verificari usoare de facut de acest tip
    float dist_y = std::abs(S.corpuri[corpA].y - S.corpuri[corpB].y);//pentru ca este putin probabil ca dintr-un numar n de corpuri, sa se ciocneasca un numar semnificativ intre ele 

    float suma_raze_x = S.corpuri[corpA].collider.bb.razaLatime + S.corpuri[corpB].collider.bb.razaLatime;
    float suma_raze_y = S.corpuri[corpA].collider.bb.razaInaltime + S.corpuri[corpB].collider.bb.razaInaltime;

    if(dist_x <= suma_raze_x && dist_y <= suma_raze_y){
        return true;
    }

    return false;
}



struct Vec2 {
    float x, y;
};

float produs_vect(Vec2 v1, Vec2 v2){
    return v1.x *v2.x + v1.y *v2.y;
}

Vec2 scade(Vec2 a, Vec2 b) { 
    return {a.x - b.x, a.y - b.y}; 
}

Vec2 aduna(Vec2 a, Vec2 b) { 
    return {a.x + b.x, a.y + b.y}; 
}

Vec2 inmulteste(Vec2 a, float s) { 
    return {a.x * s, a.y * s}; 
}

struct intersectie{
    float adancimee;
    Vec2 normala;
    bool seLovesc;
};

struct Latura {
    Vec2 p1, p2; // Cele doua capete ale laturii
    Vec2 n;      // Normala laturii
};

intersectie intersectareScaraMica(sistem &S,int corpA, int corpB){  //returneaza cat de mult patrunde un corp in altul
    
    int tipA = S.corpuri[corpA].collider.tip;
    int tipB =  S.corpuri[corpB].collider.tip;

    float x_A = S.corpuri[corpA].x;
    float x_B = S.corpuri[corpB].x;
    float y_A = S.corpuri[corpA].y;
    float y_B = S.corpuri[corpB].y;

    intersectie inter;
    inter.seLovesc = false;

    if(S.corpuri[corpA].collider.layer != S.corpuri[corpB].collider.layer)
        return inter;

    if((tipA == tipB)  && (tipA == CERC )){                     //daca verificam intersectia dintre doua cercuri trebuie doar sa vedem daca se suprapun razele
        float dist = std::sqrt( (x_A - x_B) * (x_A - x_B) + (y_A - y_B) * (y_A - y_B));
        float suma_raze = S.corpuri[corpA].collider.dimensiune1 + S.corpuri[corpB].collider.dimensiune1;

        
        inter.adancimee = dist - suma_raze;
        inter.seLovesc = (inter.adancimee >= 0);
        inter.normala.x = (x_A - x_B )/ dist;
        inter.normala.y = (y_A - y_B)/ dist;

        return inter;
    }

    if((tipA == tipB) && (tipA == DREPTUNGHI)){             //folosim Sepparating Axis Theorem pentru cele 4 axe normale la latruile celor 2 dreptunghiuri

        float phiA = S.corpuri[corpA].phi;
        float phiB = S.corpuri[corpB].phi;

        float jumatate_latime_A = S.corpuri[corpA].collider.dimensiune1 / 2.0f;
        float jumate_inaltime_A = S.corpuri[corpA].collider.dimensiune2 / 2.0f;
        float jumatate_latime_B = S.corpuri[corpB].collider.dimensiune1 / 2.0f;
        float jumate_inaltime_B = S.corpuri[corpB].collider.dimensiune2 / 2.0f;
        
        float Ai_x =  std::cos(phiA), Ai_y = std::sin(phiA);
        float Aj_x = -std::sin(phiA), Aj_y = std::cos(phiA);
        float Bi_x =  std::cos(phiB), Bi_y = std::sin(phiB);
        float Bj_x = -std::sin(phiB), Bj_y = std::cos(phiB);

        float distanta_pe_x = x_B - x_A;
        float distanta_pe_y = y_B - y_A;

        //le punem pe toate intr-un vector pt a face verificarea mai usor
        float axe_x[4] = {Ai_x, Aj_x, Bi_x, Bj_x};
        float axe_y[4] = {Ai_y, Aj_y, Bi_y, Bj_y};

        float penetrare_maxima = -999999.0f; 
        float normala_finala_x = 0.0f;
        float normala_finala_y = 0.0f;

        for (int i = 0; i < 4; i++) {   //iteram prin totatel cele 4 axe normale la dreptunghi-uri
            float axa_x = axe_x[i];
            float axa_y = axe_y[i];
        
            float distanta_proiectata = std::abs( distanta_pe_x * axa_x + distanta_pe_y * axa_y);  // vectorul dintre centrele corpurilor, proiectata pe axa curenta
            float raza_A = jumatate_latime_A * std::abs(Ai_x * axa_x + Ai_y * axa_y) + jumate_inaltime_A * std::abs(Aj_x * axa_x + Aj_y * axa_y);
            float raza_B = jumatate_latime_B * std::abs(Bi_x * axa_x + Bi_y * axa_y) + jumate_inaltime_B * std::abs(Bj_x * axa_x + Bj_y * axa_y);

            float separare = distanta_proiectata - (raza_A + raza_B);   //distante intre extremitatile corpurile pe axa curenta
        
            if(separare > 0.0f){
                return inter;
            }

            if (separare > penetrare_maxima) {      //cautam axa pe care se interpatrund cel mai putin ( valoarea fiind negativa, cautam maximul)
                penetrare_maxima = separare;
                normala_finala_x = axa_x;
                normala_finala_y = axa_y;

                if (distanta_pe_x * axa_x + distanta_pe_y * axa_y < 0.0f) {       //conventia de semne este ca vectorul normal sa arate de la A la B
                    normala_finala_x = -normala_finala_x;
                    normala_finala_y = -normala_finala_y;
            }
            }

            
    }

    inter.adancimee = -penetrare_maxima;
    inter.seLovesc = true;
    inter.normala.x = normala_finala_x;
    inter.normala.y = normala_finala_y;

    return inter;
}

    if(tipA == PUNCT)
        return inter;

    // Returneaza intersectia goala (initialized false) daca tipurile nu sunt tratate mai sus
    return inter;
}

Latura latura_optima(sistem &S, int corpA, int corpB, intersectie inter){
    Latura latura;
    
    float x = S.corpuri[corpA].x;
    float y = S.corpuri[corpA].y;
    float phi = S.corpuri[corpA].phi;
    
    // Jumatatile dimensiunilor
    float hw = S.corpuri[corpA].collider.dimensiune1 / 2.0f;
    float hh = S.corpuri[corpA].collider.dimensiune2 / 2.0f;

    // Axele locale ale corpului (directiile X si Y rotite)
    Vec2 axa_x = {std::cos(phi), std::sin(phi)};
    Vec2 axa_y = {-std::sin(phi), std::cos(phi)};

    // 1. Calculam cele 4 colturi in spatiul global
    // C0 = Dreapta-Sus, C1 = Stanga-Sus, C2 = Stanga-Jos, C3 = Dreapta-Jos
    Vec2 colturi[4];
    colturi[0] = {x + axa_x.x * hw + axa_y.x * hh, y + axa_x.y * hw + axa_y.y * hh};
    colturi[1] = {x - axa_x.x * hw + axa_y.x * hh, y - axa_x.y * hw + axa_y.y * hh};
    colturi[2] = {x - axa_x.x * hw - axa_y.x * hh, y - axa_x.y * hw - axa_y.y * hh};
    colturi[3] = {x + axa_x.x * hw - axa_y.x * hh, y + axa_x.y * hw - axa_y.y * hh};

    // 2. Normalele celor 4 laturi (Top, Left, Bottom, Right)
    Vec2 normale[4];
    normale[0] = axa_y;                 // Top (între C0 și C1)
    normale[1] = {-axa_x.x, -axa_x.y};  // Left (între C1 și C2)
    normale[2] = {-axa_y.x, -axa_y.y};  // Bottom (între C2 și C3)
    normale[3] = axa_x;                 // Right (între C3 și C0)

    float maxim = -1e10;
    int index_optim = 0;

    // 3. Cautam latura a carei normala este cea mai aliniata cu directia data
    for (int i = 0; i < 4; i++) {
        float produs = produs_vect(inter.normala, normale[i]);
        if (produs > maxim) {
            maxim = produs;
            index_optim = i;
        }
    }

    // 4. Atribuim punctele si normala in functie de indexul castigator
    latura.n = normale[index_optim];
    latura.p1 = colturi[index_optim];
    // Modulo 4 ne asigura ca dupa C3 ne intoarcem la C0
    latura.p2 = colturi[(index_optim + 1) % 4]; 

    return latura;
}

Latura latura_incidenta(sistem &S, int corpA, int corpB, intersectie inter){
    Latura laturaOptima;
    laturaOptima = latura_optima(S,corpA,corpB,inter);
    Latura latura;
    
    float x = S.corpuri[corpB].x;
    float y = S.corpuri[corpB].y;
    float phi = S.corpuri[corpB].phi;
    
    // Jumatatile dimensiunilor
    float hw = S.corpuri[corpB].collider.dimensiune1 / 2.0f;
    float hh = S.corpuri[corpB].collider.dimensiune2 / 2.0f;

    // Axele locale ale corpului (directiile X si Y rotite)
    Vec2 axa_x = {std::cos(phi), std::sin(phi)};
    Vec2 axa_y = {-std::sin(phi), std::cos(phi)};

    // 1. Calculam cele 4 colturi in spatiul global
    // C0 = Dreapta-Sus, C1 = Stanga-Sus, C2 = Stanga-Jos, C3 = Dreapta-Jos
    Vec2 colturi[4];
    colturi[0] = {x + axa_x.x * hw + axa_y.x * hh, y + axa_x.y * hw + axa_y.y * hh};
    colturi[1] = {x - axa_x.x * hw + axa_y.x * hh, y - axa_x.y * hw + axa_y.y * hh};
    colturi[2] = {x - axa_x.x * hw - axa_y.x * hh, y - axa_x.y * hw - axa_y.y * hh};
    colturi[3] = {x + axa_x.x * hw - axa_y.x * hh, y + axa_x.y * hw - axa_y.y * hh};

    // 2. Normalele celor 4 laturi (Top, Left, Bottom, Right)
    Vec2 normale[4];
    normale[0] = axa_y;                 // Top (între C0 și C1)
    normale[1] = {-axa_x.x, -axa_x.y};  // Left (între C1 și C2)
    normale[2] = {-axa_y.x, -axa_y.y};  // Bottom (între C2 și C3)
    normale[3] = axa_x;                 // Right (între C3 și C0)

    float minim = 1e10;
    int index_incidenta = 0;

    // 3. Cautam latura a carei normala este cea mai aliniata cu directia data
    for (int i = 0; i < 4; i++) {
        float produs = produs_vect(laturaOptima.n, normale[i]);
        if (produs < minim) {
            minim = produs;
            index_incidenta = i;
        }
    }

    // 4. Atribuim punctele si normala in functie de indexul castigator
    latura.n = normale[index_incidenta];
    latura.p1 = colturi[index_incidenta];
    // Modulo 4 ne asigura ca dupa C3 ne intoarcem la C0
    latura.p2 = colturi[(index_incidenta + 1) % 4]; 

    return latura;
}

// Functia generala de taiere cu un plan (rezolva ecuatiile factorului de interpolare 't')
int taierePlan(Vec2 intrari[2], int nrIntrari, Vec2 iesiri[2], Vec2 punctPlan, Vec2 normalaPlan) {
    int nrIesiri = 0;
    if (nrIntrari < 2) return 0; // Trebuie sa avem un segment valid

    // Calculam d(P) pentru ambele capete
    float d1 = produs_vect(scade(intrari[0], punctPlan), normalaPlan);
    float d2 = produs_vect(scade(intrari[1], punctPlan), normalaPlan);

    // Daca J1 are distanta pozitiva, il pastram
    if (d1 >= 0.0f) {
        iesiri[nrIesiri++] = intrari[0];
    }
    
    // Verificam daca segmentul traverseaza frontiera (d1 * d2 < 0)
    if (d1 * d2 < 0.0f) {
        // Calculam factorul de interpolare t
        float t = std::abs(d1 / (d1 - d2));
        
        // J'_1 = J_1 + t * (J_2 - J_1)
        Vec2 J_prim = aduna(intrari[0], inmulteste(scade(intrari[1], intrari[0]), t));
        iesiri[nrIesiri++] = J_prim;
    }

    // Daca J2 are distanta pozitiva, il pastram
    if (d2 >= 0.0f) {
        iesiri[nrIesiri++] = intrari[1];
    }

    return nrIesiri;
}

// Structura care va salva 1 sau 2 puncte si adancimile lor pentru a fi folosite la aplicarea impulsului
struct PuncteContact {
    Vec2 puncte[2];
    float adancimi[2];
    int nrPuncte;
};

// Functia principala descrisa la 3.4.3
PuncteContact extrageManifold(Latura ref, Latura inc) {
    PuncteContact manifold;
    manifold.nrPuncte = 0;

    // Calculam versorul tangent la latura de referinta (R1 -> R2)
    Vec2 t = scade(ref.p2, ref.p1);
    float lungime = std::sqrt(t.x * t.x + t.y * t.y);
    t.x /= lungime;
    t.y /= lungime;

    // Normala la suprafata de contact n = (-t_y, t_x)
    Vec2 n = {-t.y, t.x};

    // Vectorii pentru prelucrarea in cascada a laturii incidente J1J2
    Vec2 intrari[2] = {inc.p1, inc.p2};
    Vec2 iesiri[2];
    int nrPuncte = 2;

    // --- 1. Taierea la stanga lui R1 ---
    // Pastram d(P) = <(P - R1), t> >= 0
    nrPuncte = taierePlan(intrari, nrPuncte, iesiri, ref.p1, t);
    if (nrPuncte < 2) return manifold;

    // Pregatim datele pentru urmatoarea taiere
    intrari[0] = iesiri[0]; intrari[1] = iesiri[1];

    // --- 2. Taierea la dreapta lui R2 ---
    // Pastram d(P) = <(P - R2), -t> >= 0
    Vec2 minus_t = {-t.x, -t.y};
    nrPuncte = taierePlan(intrari, nrPuncte, iesiri, ref.p2, minus_t);
    if (nrPuncte < 2) return manifold;

    // Pregatim datele pentru taierea finala
    intrari[0] = iesiri[0]; intrari[1] = iesiri[1];
    
    // --- 3. Taierea de adancime (sub R1 R2) ---
    // Aici nu mai taiem efectiv, ci doar selectam punctele care au intrat in corp
    // Conform teoriei: d(P) = <(P - R1), n> <= 0
    for (int i = 0; i < nrPuncte; i++) {
        float adancime = produs_vect(scade(intrari[i], ref.p1), n);
        
        if (adancime <= 0.0f) { // Punctul este "sub" planul de referinta
            manifold.puncte[manifold.nrPuncte] = intrari[i];
            manifold.adancimi[manifold.nrPuncte] = -adancime; // Salvam valoarea pozitiva pentru corectia de pozitie
            manifold.nrPuncte++;
        }
    }

    return manifold;
}

void ciocnire(sistem &S, int corpA, int corpB, intersectie inter){
   
}
