#include "colliziune.h"

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

    if (S.corpuri[corpA].collider.cadru != S.corpuri[corpB].collider.cadru && (S.corpuri[corpA].collider.obiectVirtual ==0 && S.corpuri[corpB].collider.obiectVirtual == 0))    //conditia de a se afla pe acelasi cadru nu se aplica pentru corpurile virtuale
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

    if ((tipA == DREPTUNGHI && tipB == CERC) || (tipA == CERC && tipB == DREPTUNGHI))
    {
        // Stabilim exact cine e cercul si cine e dreptunghiul
        int idRect = (tipA == DREPTUNGHI) ? corpA : corpB;
        int idCirc = (tipA == CERC) ? corpA : corpB;

        float R = S.corpuri[idCirc].collider.dimensiune1;
        float hw = S.corpuri[idRect].collider.dimensiune1 / 2.0f;
        float hh = S.corpuri[idRect].collider.dimensiune2 / 2.0f;

        // Vectorul de la dreptunghi la cerc (in coordonate globale)
        float cx = S.corpuri[idCirc].x - S.corpuri[idRect].x;
        float cy = S.corpuri[idCirc].y - S.corpuri[idRect].y;
        float phi = S.corpuri[idRect].phi;

        // 1. Rotim centrul cercului in sistemul local al dreptunghiului (rotatie inversa)
        float cos_phi = std::cos(phi);
        float sin_phi = std::sin(phi);
        float localX = cx * cos_phi + cy * sin_phi;
        float localY = -cx * sin_phi + cy * cos_phi;

        // 2. Gasim cel mai apropiat punct de pe dreptunghi (clamping)
        float closestX = std::max(-hw, std::min(hw, localX));
        float closestY = std::max(-hh, std::min(hh, localY));

        // 3. Calculam distanta de la acel punct la centrul cercului
        float dx = localX - closestX;
        float dy = localY - closestY;
        float distSq = dx * dx + dy * dy;

        // Daca e mai mica decat raza la patrat, avem coliziune!
        if (distSq < R * R)
        {
            float dist = std::sqrt(distSq);
            float localNx, localNy;

            // Caz special: centrul cercului a patruns complet in interiorul dreptunghiului
            if (dist == 0.0f) 
            {
                float dLeft = localX - (-hw);
                float dRight = hw - localX;
                float dBottom = localY - (-hh);
                float dTop = hh - localY;

                float minDist = dLeft;
                localNx = -1.0f; localNy = 0.0f;

                if (dRight < minDist) { minDist = dRight; localNx = 1.0f; localNy = 0.0f; }
                if (dTop < minDist)   { minDist = dTop;   localNx = 0.0f; localNy = 1.0f; }
                if (dBottom < minDist){ minDist = dBottom;localNx = 0.0f; localNy = -1.0f; }

                inter.adancimee = R + minDist;
            }
            else
            {
                // Coliziune normala (pe margine)
                inter.adancimee = R - dist;
                localNx = dx / dist;
                localNy = dy / dist;
            }

            // 4. Rotim normala inapoi in sistemul global
            float globalNx = localNx * cos_phi - localNy * sin_phi;
            float globalNy = localNx * sin_phi + localNy * cos_phi;

            // 5. Conventie: Normala trebuie sa arate mereu de la A spre B
            if (tipA == CERC) {
                globalNx = -globalNx;
                globalNy = -globalNy;
            }

            inter.seLovesc = true;
            inter.normala.x = globalNx;
            inter.normala.y = globalNy;
            return inter;
        }
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

    int tipA = A.collider.tip;
    int tipB = B.collider.tip;

    // Normala de la SAT arata mereu de la A spre B
    Vec2 normala_A = inter.normala;
    Vec2 normala_B = {-inter.normala.x, -inter.normala.y};

    PuncteContact contacte;
    contacte.nrPuncte = 0;

    if(tipA == DREPTUNGHI && tipB == DREPTUNGHI)
    {
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
    contacte = extrageManifold(laturaReferinta, laturaIncidenta);
    }
    else if (tipA == CERC && tipB == CERC) 
    {
        // Doua cercuri se ating mereu intr-un singur punct pe linia centrelor
        contacte.nrPuncte = 1;
        contacte.adancimi[0] = inter.adancimee;
        // Punctul de contact se afla pe marginea lui A, inspre B
        contacte.puncte[0].x = A.x + normala_A.x * A.collider.dimensiune1;
        contacte.puncte[0].y = A.y + normala_A.y * A.collider.dimensiune1;
    }
    else 
    {
        // Coliziune mixta: CERC - DREPTUNGHI
        contacte.nrPuncte = 1;
        contacte.adancimi[0] = inter.adancimee;
        
        if (tipA == CERC) {
            // A este cercul. Punctul de pe el este pe directia normalei (spre B)
            contacte.puncte[0].x = A.x + normala_A.x * A.collider.dimensiune1;
            contacte.puncte[0].y = A.y + normala_A.y * A.collider.dimensiune1;
        } else {
            // B este cercul. Punctul de pe el este in directie opusa normalei globale (spre A)
            contacte.puncte[0].x = B.x - normala_A.x * B.collider.dimensiune1;
            contacte.puncte[0].y = B.y - normala_A.y * B.collider.dimensiune1;
        }
    }
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
    // 2. CALCULUL PERCUTIEI 'P'
    // ==============================================================
    float k = A.material.restituire * B.material.restituire; // Preluam coeficientul din sistem

    float n_x = inter.normala.x;
    float n_y = inter.normala.y;

    for (int i = 0; i < contacte.nrPuncte; i++)
    {
        float x_A = contacte.puncte[i].x - A.x;
        float y_A = contacte.puncte[i].y - A.y;
        float x_B = contacte.puncte[i].x - B.x;
        float y_B = contacte.puncte[i].y - B.y;

        float termen_rot_A = (x_A * n_y - y_A * n_x);
        float termen_rot_B = (x_B * n_y - y_B * n_x);

        float v_CA_n = A.v_x * n_x + A.v_y * n_y + A.omega * termen_rot_A;
        float v_CB_n = B.v_x * n_x + B.v_y * n_y + B.omega * termen_rot_B;

        float v_rel_n = v_CB_n - v_CA_n;

        // ELIMINAT: Am sters "if (v_rel_n > 0.0f) continue;" pentru a lasa corectia sa functioneze

        float numitor = invM_A + invM_B + (termen_rot_A * termen_rot_A) * invI_A + (termen_rot_B * termen_rot_B) * invI_B;

        // NOU: Introducem corectia de penetrare ca un bias de viteza (Baumgarte pentru contacte)
        float v_bias = 20.0f * std::max(contacte.adancimi[i] - 0.005f, 0.0f);

        // Adaugam v_bias direct in formula percutiei P
        float P = -(1.0f + k) * v_rel_n + v_bias;
        P /= (numitor * (float)contacte.nrPuncte);

        // Taiem percutiile negative ca poligoanele sa nu se "lipeasca" intre ele cand se despart
        if (P < 0.0f) P = 0.0f;

        // Aplicarea percutiei P direct pe viteze
        A.v_x -= P * n_x * invM_A;
        A.v_y -= P * n_y * invM_A;
        A.omega -= P * termen_rot_A * invI_A;

        B.v_x += P * n_x * invM_B;
        B.v_y += P * n_y * invM_B;
        B.omega += P * termen_rot_B * invI_B;
        
        // ... RESTUL FUNCTIEI (frecarea etc.) RAMANE ABSOLUT LA FEL

        // ==============================================================
        // 4. CALCULUL FRECARIE DE ALUNECARE
        // ==============================================================
        
        float mu_s = std::sqrt(A.material.frecareStatica*B.material.frecareStatica);   // Coeficient de frecare statica
        float mu_d = std::sqrt(A.material.frecareDinamica*B.material.frecareDinamica);  // Coeficientul de frecare dinamica

        float t_x = -n_y;               //vectorul tangent la ciocnire
        float t_y = n_x;

        float termen_rot_At = (x_A * t_y - y_A * t_x);
        float termen_rot_Bt = (x_B * t_y - y_B * t_x);

        float v_CA_t = A.v_x * t_x + A.v_y * t_y + A.omega * termen_rot_At;
        float v_CB_t = B.v_x * t_x + B.v_y * t_y + B.omega * termen_rot_Bt;

        float v_rel_t = v_CB_t - v_CA_t;

        float numitor_t = invM_A + invM_B +
                          (termen_rot_At * termen_rot_At) * invI_A +
                          (termen_rot_Bt * termen_rot_Bt) * invI_B;

        
        float F_f_teoretic = -v_rel_t / (numitor_t * (float)contacte.nrPuncte);
        float F_f_final;

        if(std::abs(F_f_teoretic) <= P *mu_s){      //verificam daca aplicam frecarea statica sau cea dinamica
            F_f_final = F_f_teoretic;
        } else {
            F_f_final = (F_f_teoretic > 0.0f ? 1.0f : -1.0f) * P *mu_d;
        }

        A.v_x -= F_f_final * t_x * invM_A;
        A.v_y -= F_f_final * t_y * invM_A;
        A.omega -= F_f_final * termen_rot_At * invI_A;

        B.v_x += F_f_final * t_x * invM_B;
        B.v_y += F_f_final * t_y * invM_B;
        B.omega += F_f_final * termen_rot_Bt * invI_B;

        // ==============================================================
        // 5. CALCULUL FRECARIE LA ROSTOGOLIRE
        // ==============================================================

        float s_s = std::sqrt(A.material.frecareRostogolireStatica*B.material.frecareRostogolireStatica);
        float s_d = std::sqrt(A.material.frecareRostogolireDinamica*B.material.frecareRostogolireDinamica);

        float d_omega = B.omega - A.omega;
        float invI_suma = invI_A + invI_B;

        if(invI_suma > 0.0001f){
            float P_ung_teoretic = -d_omega / (invI_suma *(float)contacte.nrPuncte);
            float P_ung_final;

        if(std::abs(P_ung_teoretic) <= P* s_s){
            P_ung_final = P_ung_teoretic;
        } else {
            P_ung_final = (P_ung_teoretic > 0.0f ? 1.0f : -1.0f) * P * s_d;
        }

        A.omega -= P_ung_final *invI_A;
        B.omega += P_ung_final *invI_B;
        
        }
    }

S.incarcaStare();
}

void percutiiDeLegatura(sistem &S){                   //rezolva sistemul (J * A^-1 * J^T) Lambda_perc = - J * q*punct
    
    if(S.p == 0) return;

    matrice Lambda_perc(S.p, 1);
    matrice M(S.p,S.p);
    matrice L(S.p,S.p);

    matrice J_T = S.J_F ^ "T";

    M =  S.J_F * S.A_inv * J_T;                   // matricea din partea stanga a sistemului

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

    int nr_corpuri = S.corpuri.size();
    matrice q(nr_corpuri*3,1);

    for(int i = 0; i < nr_corpuri; i++){
        q(i*3 + 0 ,0) = S.corpuri[i].v_x;
        q(i*3 + 1 ,0) = S.corpuri[i].v_y;
        q(i*3 + 2 ,0) = S.corpuri[i].omega;
    }

    matrice y(S.p,1);
    matrice B(S.p,1);
    B = - S.J_F * q;   //matricea din partea dreapta a sistemului

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
            suma += L(j,i)* Lambda_perc(j,0);          // L(j,i) este L^T(i,j)
        } 
        Lambda_perc(i,0) = (y(i,0) - suma) / L(i,i);
    }

    matrice Delta_q = S.A_inv * J_T *Lambda_perc;

    for(int i = 0; i < nr_corpuri; i++) {
        if (S.corpuri[i].M > 1e10f) continue; // Corpurile fixe nu se misca

        S.corpuri[i].v_x   += Delta_q(i * 3 + 0, 0);
        S.corpuri[i].v_y   += Delta_q(i * 3 + 1, 0);
        S.corpuri[i].omega += Delta_q(i * 3 + 2, 0);
    }
}

void verificarCiocniri(sistem &S)
{
    S.corpuriSelectate.clear();
    bool aFostCiocnire = 0;

    for (int i = 0; i < S.corpuri.size(); i++)
    {
        S.corpuri[i].seteazaBoundingBox();
    }

    for (int i = 0; i < S.corpuri.size(); i++)
    {
        S.corpuri[i].collider.selectat = 0;                       //la fiecare frame resetam starea de "selectat" al obiectelor
    
        for (int j = i + 1; j < S.corpuri.size(); j++)
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
                    else{
                        if (S.mod_curent == 0) { 
                            ciocnire(S, i, j, inter);
                            aFostCiocnire = true;
                        }
                    }
                }
            }
        }
    }

    if(aFostCiocnire)
        percutiiDeLegatura(S);
}