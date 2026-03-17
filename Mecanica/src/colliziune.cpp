#pragma once
#include "sistem.h"
#include "matrice.h"
#include <cmath>

bool intersectareScaraLarga(sistem &S, int corpA, int corpB)
{ // verifica doar daca cutiile in care sunt bagate corpurile se intersecteaza
  // doar daca cutiile corpurile se supran exista posibilitatea ca ele sa se intersecteze cu adevarat, caz in care facem o verificare mai exacta
    float dist_x = std::abs(S.corpuri[corpA].x - S.corpuri[corpB].x); // costul computational de a face doua verificari in cazul in care se supranul, este mult justificat de timpul castigat prin calculul a multor verificari usoare de facut de acest tip
    float dist_y = std::abs(S.corpuri[corpA].y - S.corpuri[corpB].y); // pentru ca este putin probabil ca dintr-un numar n de corpuri, sa se ciocneasca un numar semnificativ intre ele

    float suma_raze_x = S.corpuri[corpA].collider.bb.razaLatime + S.corpuri[corpB].collider.bb.razaLatime;
    float suma_raze_y = S.corpuri[corpA].collider.bb.razaInaltime + S.corpuri[corpB].collider.bb.razaInaltime;

    if (dist_x <= suma_raze_x && dist_y <= suma_raze_y)
    {
        return true;
    }

    return false;
}

struct Vec2
{
    float x, y;
};

float produs_vect(Vec2 v1, Vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

Vec2 scade(Vec2 a, Vec2 b)
{
    return {a.x - b.x, a.y - b.y};
}

Vec2 aduna(Vec2 a, Vec2 b)
{
    return {a.x + b.x, a.y + b.y};
}

Vec2 inmulteste(Vec2 a, float s)
{
    return {a.x * s, a.y * s};
}

struct intersectie
{
    float adancimee;
    Vec2 normala;
    bool seLovesc;
};

struct Latura
{
    Vec2 p1, p2; // Cele doua capete ale laturii
    Vec2 n;      // Normala laturii
};

intersectie intersectareScaraMica(sistem &S, int corpA, int corpB)
{ // returneaza cat de mult patrunde un corp in altul

    int tipA = S.corpuri[corpA].collider.tip;
    int tipB = S.corpuri[corpB].collider.tip;

    float x_A = S.corpuri[corpA].x;
    float x_B = S.corpuri[corpB].x;
    float y_A = S.corpuri[corpA].y;
    float y_B = S.corpuri[corpB].y;

    intersectie inter;
    inter.seLovesc = false;

    if (S.corpuri[corpA].collider.layer != S.corpuri[corpB].collider.layer && (S.corpuri[corpA].collider.obiectVirtual ==0 && S.corpuri[corpB].collider.obiectVirtual == 0))    //conditia de a se afla pe acelasi layer nu se aplica pentru corpurile virtuale
        return inter;    

    if ((tipA == tipB) && (tipA == CERC))
    { // daca verificam intersectia dintre doua cercuri trebuie doar sa vedem daca se suprapun razele
        float dist = std::sqrt((x_A - x_B) * (x_A - x_B) + (y_A - y_B) * (y_A - y_B));
        float suma_raze = S.corpuri[corpA].collider.dimensiune1 + S.corpuri[corpB].collider.dimensiune1;

        inter.adancimee = dist - suma_raze;
        inter.seLovesc = (inter.adancimee >= 0);
        inter.normala.x = (x_A - x_B) / dist;
        inter.normala.y = (y_A - y_B) / dist;

        return inter;
    }

    if ((tipA == tipB) && (tipA == DREPTUNGHI))
    { // folosim Sepparating Axis Theorem pentru cele 4 axe normale la latruile celor 2 dreptunghiuri

        float phiA = S.corpuri[corpA].phi;
        float phiB = S.corpuri[corpB].phi;

        float jumatate_latime_A = S.corpuri[corpA].collider.dimensiune1 / 2.0f;
        float jumate_inaltime_A = S.corpuri[corpA].collider.dimensiune2 / 2.0f;
        float jumatate_latime_B = S.corpuri[corpB].collider.dimensiune1 / 2.0f;
        float jumate_inaltime_B = S.corpuri[corpB].collider.dimensiune2 / 2.0f;

        float Ai_x = std::cos(phiA), Ai_y = std::sin(phiA);
        float Aj_x = -std::sin(phiA), Aj_y = std::cos(phiA);
        float Bi_x = std::cos(phiB), Bi_y = std::sin(phiB);
        float Bj_x = -std::sin(phiB), Bj_y = std::cos(phiB);

        float distanta_pe_x = x_B - x_A;
        float distanta_pe_y = y_B - y_A;

        // le punem pe toate intr-un vector pt a face verificarea mai usor
        float axe_x[4] = {Ai_x, Aj_x, Bi_x, Bj_x};
        float axe_y[4] = {Ai_y, Aj_y, Bi_y, Bj_y};

        float penetrare_maxima = -999999.0f;
        float normala_finala_x = 0.0f;
        float normala_finala_y = 0.0f;

        for (int i = 0; i < 4; i++)
        { // iteram prin totatel cele 4 axe normale la dreptunghi-uri
            float axa_x = axe_x[i];
            float axa_y = axe_y[i];

            float distanta_proiectata = std::abs(distanta_pe_x * axa_x + distanta_pe_y * axa_y); // vectorul dintre centrele corpurilor, proiectata pe axa curenta
            float raza_A = jumatate_latime_A * std::abs(Ai_x * axa_x + Ai_y * axa_y) + jumate_inaltime_A * std::abs(Aj_x * axa_x + Aj_y * axa_y);
            float raza_B = jumatate_latime_B * std::abs(Bi_x * axa_x + Bi_y * axa_y) + jumate_inaltime_B * std::abs(Bj_x * axa_x + Bj_y * axa_y);

            float separare = distanta_proiectata - (raza_A + raza_B); // distante intre extremitatile corpurile pe axa curenta

            if (separare > 0.0f)
            {
                return inter;
            }

            if (separare > penetrare_maxima)
            { // cautam axa pe care se interpatrund cel mai putin ( valoarea fiind negativa, cautam maximul)
                penetrare_maxima = separare;
                normala_finala_x = axa_x;
                normala_finala_y = axa_y;

                if (distanta_pe_x * axa_x + distanta_pe_y * axa_y < 0.0f)
                { // conventia de semne este ca vectorul normal sa arate de la A la B
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

    if(tipA == DREPTUNGHI && tipB == CERC){
        
    }

    if (tipA == PUNCT)
        return inter;

    // Returneaza intersectia goala (initialized false) daca tipurile nu sunt tratate mai sus
    return inter;
}

Latura gasesteFataSuport(sistem &S, int corp, Vec2 directie)
{
    Latura latura;

    float x = S.corpuri[corp].x;
    float y = S.corpuri[corp].y;
    float phi = S.corpuri[corp].phi;

    float hw = S.corpuri[corp].collider.dimensiune1 / 2.0f;
    float hh = S.corpuri[corp].collider.dimensiune2 / 2.0f;

    Vec2 axa_x = {std::cos(phi), std::sin(phi)};
    Vec2 axa_y = {-std::sin(phi), std::cos(phi)};

    Vec2 colturi[4];
    colturi[0] = {x + axa_x.x * hw + axa_y.x * hh, y + axa_x.y * hw + axa_y.y * hh};
    colturi[1] = {x - axa_x.x * hw + axa_y.x * hh, y - axa_x.y * hw + axa_y.y * hh};
    colturi[2] = {x - axa_x.x * hw - axa_y.x * hh, y - axa_x.y * hw - axa_y.y * hh};
    colturi[3] = {x + axa_x.x * hw - axa_y.x * hh, y + axa_x.y * hw - axa_y.y * hh};

    Vec2 normale[4];
    normale[0] = axa_y;
    normale[1] = {-axa_x.x, -axa_x.y};
    normale[2] = {-axa_y.x, -axa_y.y};
    normale[3] = axa_x;

    float maxim = -1e10f;
    int index_optim = 0;

    // Cautam fata care arata in "directie"
    for (int i = 0; i < 4; i++)
    {
        float produs = produs_vect(directie, normale[i]);
        if (produs > maxim)
        {
            maxim = produs;
            index_optim = i;
        }
    }

    latura.n = normale[index_optim];
    latura.p1 = colturi[index_optim];
    latura.p2 = colturi[(index_optim + 1) % 4];

    return latura;
}

// Functia generala de taiere cu un plan (rezolva ecuatiile factorului de interpolare 't')
int taierePlan(Vec2 intrari[2], int nrIntrari, Vec2 iesiri[2], Vec2 punctPlan, Vec2 normalaPlan)
{
    int nrIesiri = 0;
    if (nrIntrari < 2)
        return 0; // Trebuie sa avem un segment valid

    // Calculam d(P) pentru ambele capete
    float d1 = produs_vect(scade(intrari[0], punctPlan), normalaPlan);
    float d2 = produs_vect(scade(intrari[1], punctPlan), normalaPlan);

    // Daca J1 are distanta pozitiva, il pastram
    if (d1 >= 0.0f)
    {
        iesiri[nrIesiri++] = intrari[0];
    }

    // Verificam daca segmentul traverseaza frontiera (d1 * d2 < 0)
    if (d1 * d2 < 0.0f)
    {
        // Calculam factorul de interpolare t
        float t = std::abs(d1 / (d1 - d2));

        // J'_1 = J_1 + t * (J_2 - J_1)
        Vec2 J_prim = aduna(intrari[0], inmulteste(scade(intrari[1], intrari[0]), t));
        iesiri[nrIesiri++] = J_prim;
    }

    // Daca J2 are distanta pozitiva, il pastram
    if (d2 >= 0.0f)
    {
        iesiri[nrIesiri++] = intrari[1];
    }

    return nrIesiri;
}

// Structura care va salva 1 sau 2 puncte si adancimile lor pentru a fi folosite la aplicarea impulsului
struct PuncteContact
{
    Vec2 puncte[2];
    float adancimi[2];
    int nrPuncte;
};

PuncteContact extrageManifold(Latura ref, Latura inc)
{
    PuncteContact manifold;
    manifold.nrPuncte = 0;

    // Calculam versorul tangent la latura de referinta (R1 -> R2)
    Vec2 t = scade(ref.p2, ref.p1);
    float lungime = std::sqrt(t.x * t.x + t.y * t.y);
    t.x /= lungime;
    t.y /= lungime;

    // Normala la suprafata de contact n
    Vec2 n = ref.n;

    // Vectorii pentru prelucrarea in cascada a laturii incidente J1J2
    Vec2 intrari[2] = {inc.p1, inc.p2};
    Vec2 iesiri[2];
    int nrPuncte = 2;

    // --- 1. Taierea la stanga lui R1 ---
    // Pastram d(P) = <(P - R1), t> >= 0
    nrPuncte = taierePlan(intrari, nrPuncte, iesiri, ref.p1, t);
    if (nrPuncte < 2)
        return manifold;

    // Pregatim datele pentru urmatoarea taiere
    intrari[0] = iesiri[0];
    intrari[1] = iesiri[1];

    // --- 2. Taierea la dreapta lui R2 ---
    // Pastram d(P) = <(P - R2), -t> >= 0
    Vec2 minus_t = {-t.x, -t.y};
    nrPuncte = taierePlan(intrari, nrPuncte, iesiri, ref.p2, minus_t);
    if (nrPuncte < 2)
        return manifold;

    // Pregatim datele pentru taierea finala
    intrari[0] = iesiri[0];
    intrari[1] = iesiri[1];

    // --- 3. Taierea de adancime (sub R1 R2) ---
    // Aici nu mai taiem efectiv, ci doar selectam punctele care au intrat in corp
    // Conform teoriei: d(P) = <(P - R1), n> <= 0
    for (int i = 0; i < nrPuncte; i++)
    {
        float adancime = produs_vect(scade(intrari[i], ref.p1), n);

        if (adancime <= 0.0f)
        { // Punctul este "sub" planul de referinta
            manifold.puncte[manifold.nrPuncte] = intrari[i];
            manifold.adancimi[manifold.nrPuncte] = -adancime; // Salvam valoarea pozitiva pentru corectia de pozitie
            manifold.nrPuncte++;
        }
    }

    return manifold;
}

void ciocnire(sistem &S, int corpA, int corpB, intersectie inter)
{

    rigid &A = S.corpuri[corpA];
    rigid &B = S.corpuri[corpB];

    // Normala de la SAT arata mereu de la A spre B
    Vec2 normala_A = inter.normala;
    Vec2 normala_B = {-inter.normala.x, -inter.normala.y};

    // 1. Gasim cele mai bune fete pentru ambele corpuri
    Latura fataA = gasesteFataSuport(S, corpA, normala_A);
    Latura fataB = gasesteFataSuport(S, corpB, normala_B);

    // 2. Determinam care este "Latura de Referinta"
    // Referinta e fata cea mai "perpendiculara" pe impact (produs scalar cel mai mare / aproape de 1)
    float dotA = produs_vect(fataA.n, normala_A);
    float dotB = produs_vect(fataB.n, normala_B);

    Latura laturaReferinta, laturaIncidenta;

    // O mica toleranta pentru a favoriza corpul A la egalitate (ca sa nu "fluture" intre decizii)
    if (dotB > dotA + 0.001f)
    {
        laturaReferinta = fataB;
        laturaIncidenta = fataA;
    }
    else
    {
        laturaReferinta = fataA;
        laturaIncidenta = fataB;
    }

    // 3. Acum extragem punctele. Decuparea va functiona intotdeauna perfect!
    PuncteContact contacte = extrageManifold(laturaReferinta, laturaIncidenta);

    // Daca totusi nu exista contact (frecare/margini limitrofe), evitam aplicarea impulsului
    if (contacte.nrPuncte == 0)
        return;
    // Inversul maselor si inertiilor (pentru obiecte statice se considera 0)
    float invM_A = (A.M > 1e10f) ? 0.0f : 1.0f / A.M;
    float invM_B = (B.M > 1e10f) ? 0.0f : 1.0f / B.M;
    float invI_A = (A.J > 1e10f) ? 0.0f : 1.0f / A.J;
    float invI_B = (B.J > 1e10f) ? 0.0f : 1.0f / B.J;

    float sumaMaseInverse = invM_A + invM_B;
    if (sumaMaseInverse == 0.0f)
        return;

    // ==============================================================
    // 2. CORECTIA DE POZITIE (Pentru a nu se scufunda corpurile)
    // ==============================================================
    float adancime_maxima = 0.0f;
    for (int i = 0; i < contacte.nrPuncte; i++)
    {
        if (contacte.adancimi[i] > adancime_maxima)
        {
            adancime_maxima = contacte.adancimi[i];
        }
    }

    const float joc = 0.01f;             // are rol in stabilizare, "jocul" admisibil pentru ca corpurile sa fie considerate una peste altele, ca sa nu tremure la simulare
    const float procent_corectie = 0.2f; // cat de mult sa scoata afara corpurile unul din altul, daca il lasam la 1.0f, e posibil sa introduca acceleratii mari in cazuri cu mai multe corpuri

    float corectie = std::max(adancime_maxima - joc, 0.0f) * procent_corectie;
    float mutare_A = corectie * (invM_A / sumaMaseInverse);
    float mutare_B = corectie * (invM_B / sumaMaseInverse);

    // Mutam centrele de masa pe directia normalei
    A.x -= inter.normala.x * mutare_A;
    A.y -= inter.normala.y * mutare_A;
    B.x += inter.normala.x * mutare_B;
    B.y += inter.normala.y * mutare_B;

    // ==============================================================
    // 3. CALCULUL PERCUTIEI 'P'
    // ==============================================================
    float k = S.coeficientRestituire(corpA, corpB); // Preluam coeficientul din sistem
    float mu = S.coeficientFrecare(corpA, corpB);   // Coeficient de frecare

    float n_x = inter.normala.x;
    float n_y = inter.normala.y;

    for (int i = 0; i < contacte.nrPuncte; i++)
    {
        // Coordonatele relative ale punctului de contact fata de centrul de masa
        // Acestea sunt x_A, y_A si x_B, y_B din demonstratia ta matematica
        float x_A = contacte.puncte[i].x - A.x;
        float y_A = contacte.puncte[i].y - A.y;
        float x_B = contacte.puncte[i].x - B.x;
        float y_B = contacte.puncte[i].y - B.y;

        // Termenii de moment: (x * n_y - y * n_x)
        float termen_rot_A = (x_A * n_y - y_A * n_x);
        float termen_rot_B = (x_B * n_y - y_B * n_x);

        // Proiectiile vitezelor pe normala (v_CA,n' si v_CB,n')
        float v_CA_n = A.v_x * n_x + A.v_y * n_y + A.omega * termen_rot_A;
        float v_CB_n = B.v_x * n_x + B.v_y * n_y + B.omega * termen_rot_B;

        // Diferenta vitezelor pe normala
        float v_rel_n = v_CB_n - v_CA_n;

        // Daca se indeparteaza deja, sarim peste percutie
        if (v_rel_n > 0.0f)
            continue;

        // Calculul numitorului ecuatiei (masa efectiva pe normala)
        float numitor = invM_A + invM_B + (termen_rot_A * termen_rot_A) * invI_A + (termen_rot_B * termen_rot_B) * invI_B;

        // Calculul final al percutiei P
        float P = -(1.0f + k) * v_rel_n;
        P /= (numitor * (float)contacte.nrPuncte); // Impartim daca avem mai multe puncte de contact

        // Aplicarea percutiei P direct pe viteze
        A.v_x -= P * n_x * invM_A;
        A.v_y -= P * n_y * invM_A;
        A.omega -= P * termen_rot_A * invI_A;

        B.v_x += P * n_x * invM_B;
        B.v_y += P * n_y * invM_B;
        B.omega += P * termen_rot_B * invI_B;

        // ==============================================================
        // 4. CALCULUL FRECARII (Asemanator, dar pe directia tangentei)
        // ==============================================================
        float t_x = -n_y;
        float t_y = n_x;

        float termen_rot_At = (x_A * t_y - y_A * t_x);
        float termen_rot_Bt = (x_B * t_y - y_B * t_x);

        float v_CA_t = A.v_x * t_x + A.v_y * t_y + A.omega * termen_rot_At;
        float v_CB_t = B.v_x * t_x + B.v_y * t_y + B.omega * termen_rot_Bt;

        float v_rel_t = v_CB_t - v_CA_t;

        float numitor_t = invM_A + invM_B +
                          (termen_rot_At * termen_rot_At) * invI_A +
                          (termen_rot_Bt * termen_rot_Bt) * invI_B;

        float P_t = -v_rel_t / (numitor_t * (float)contacte.nrPuncte);

        // Plafonam frecarea sa nu depaseasca limita Coulomb (mu * forta normala P)
        if (std::abs(P_t) > P * mu)
        {
            P_t = (P_t > 0.0f ? 1.0f : -1.0f) * P * mu;
        }

        // Aplicarea percutiei de frecare P_t
        A.v_x -= P_t * t_x * invM_A;
        A.v_y -= P_t * t_y * invM_A;
        A.omega -= P_t * termen_rot_At * invI_A;

        B.v_x += P_t * t_x * invM_B;
        B.v_y += P_t * t_y * invM_B;
        B.omega += P_t * termen_rot_Bt * invI_B;
    }
}

void verificarCiocniri(sistem &S)
{
    S.corpuriSelectate.clear();

    for (int i = 0; i < S.nr_corpuri; i++)
    {
        S.corpuri[i].seteazaBoundingBox();
    }

    for (int i = 0; i < S.nr_corpuri; i++)
    {
        S.corpuri[i].collider.selectat = 0;                       //la fiecare frame resetam starea de "selectat" al obiectelor
    
        for (int j = i + 1; j < S.nr_corpuri; j++)
        {

            if (S.corpuri[i].M > 1e10f && S.corpuri[j].M > 1e10f) // daca luam doi pereti, nu incercam sa calculam ciocnirea dintre ei
                continue;

            if (intersectareScaraLarga(S, i, j))
            {
                intersectie inter = intersectareScaraMica(S, i, j);
                if (inter.seLovesc)
                {
                    if(S.corpuri[i].collider.obiectVirtual == 1){
                        S.corpuri[j].collider.selectat = 1;
                        S.corpuriSelectate.push_back(j);
                    }
                    else if(S.corpuri[j].collider.obiectVirtual == 1){
                        S.corpuri[i].collider.selectat = 1;
                        S.corpuriSelectate.push_back(i);
                    }
                    else
                        ciocnire(S, i, j, inter);
                }
            }
        }
    }
}