#include "colliziune.h"

float percutie_maxima = 5000.0f;

bool intersectareScaraLarga(sistem &S, int corpA, int corpB)
{ // verifica doar daca cutiile in care sunt bagate corpurile se intersecteaza
    // doar daca cutiile corpurile se supran exista posibilitatea ca ele sa se intersecteze cu adevarat, caz in care facem o verificare mai exacta
    float dist_x = std::abs(S.corpuri[corpA].pozitie.x - S.corpuri[corpB].pozitie.x); // costul computational de a face doua verificari in cazul in care se supranul, este mult justificat de timpul castigat prin calculul a multor verificari usoare de facut de acest tip
    float dist_y = std::abs(S.corpuri[corpA].pozitie.y - S.corpuri[corpB].pozitie.y); // pentru ca este putin probabil ca dintr-un numar n de corpuri, sa se ciocneasca un numar semnificativ intre ele
    
    float suma_raze_x = S.corpuri[corpA].collider.bb.razaLatime + S.corpuri[corpB].collider.bb.razaLatime;
    float suma_raze_y = S.corpuri[corpA].collider.bb.razaInaltime + S.corpuri[corpB].collider.bb.razaInaltime;
    
    if (dist_x <= suma_raze_x && dist_y <= suma_raze_y)
    {
        return true;
    }

    return false;
}

intersectie intersectareScaraMica(sistem &S, int corpA, int corpB)
{ // returneaza cat de mult patrunde un corp in altul
    
    int tipA = S.corpuri[corpA].collider.tip;
    int tipB = S.corpuri[corpB].collider.tip;
    
    float x_A = S.corpuri[corpA].pozitie.x;
    float x_B = S.corpuri[corpB].pozitie.x;
    float y_A = S.corpuri[corpA].pozitie.y;
    float y_B = S.corpuri[corpB].pozitie.y;
    
    intersectie inter;
    inter.seLovesc = false;
    
    if (S.corpuri[corpA].collider.cadru != S.corpuri[corpB].collider.cadru && (S.corpuri[corpA].collider.obiectVirtual ==0 && S.corpuri[corpB].collider.obiectVirtual == 0))    //conditia de a se afla pe acelasi cadru nu se aplica pentru corpurile virtuale
    return inter;    
    
    if ((tipA == tipB) && (tipA == CERC))
    { // daca verificam intersectia dintre doua cercuri trebuie doar sa vedem daca se suprapun razele
        
        float dist = std::sqrt((x_A - x_B) * (x_A - x_B) + (y_A - y_B) * (y_A - y_B));
        float suma_raze = S.corpuri[corpA].collider.dimensiune1 + S.corpuri[corpB].collider.dimensiune1;
        
        inter.adancimee = suma_raze - dist;
        inter.seLovesc = (inter.adancimee >= 0);
        
        if (dist > 0.0001f) {
            inter.normala.x = (x_B - x_A) / dist;
            inter.normala.y = (y_B - y_A) / dist;
        } else {
            inter.normala.x = 1.0f; 
            inter.normala.y = 0.0f;
        }
        
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
        float cx = S.corpuri[idCirc].pozitie.x - S.corpuri[idRect].pozitie.x;
        float cy = S.corpuri[idCirc].pozitie.y - S.corpuri[idRect].pozitie.y;
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

Latura gasesteFataSuport(sistem &S, int corp, vec2 directie)
{
    Latura latura;
    
    float x = S.corpuri[corp].pozitie.x;
    float y = S.corpuri[corp].pozitie.y;
    float phi = S.corpuri[corp].phi;
    
    float hw = S.corpuri[corp].collider.dimensiune1 / 2.0f;
    float hh = S.corpuri[corp].collider.dimensiune2 / 2.0f;
    
    vec2 axa_x(std::cos(phi), std::sin(phi));
    vec2 axa_y(-std::sin(phi), std::cos(phi));
    
    vec2 colturi[4];
    colturi[0] = vec2(x + axa_x.x * hw + axa_y.x * hh, y + axa_x.y * hw + axa_y.y * hh);
    colturi[1] = vec2(x - axa_x.x * hw + axa_y.x * hh, y - axa_x.y * hw + axa_y.y * hh);
    colturi[2] = vec2(x - axa_x.x * hw - axa_y.x * hh, y - axa_x.y * hw - axa_y.y * hh);
    colturi[3] = vec2(x + axa_x.x * hw - axa_y.x * hh, y + axa_x.y * hw - axa_y.y * hh);
    
    vec2 normale[4];
    normale[0] = axa_y;
    normale[1] = vec2(-axa_x.x, -axa_x.y);
    normale[2] = vec2(-axa_y.x, -axa_y.y);
    normale[3] = axa_x;
    
    float maxim = -1e10f;
    int index_optim = 0;
    
    // Cautam fata care arata in "directie"
    for (int i = 0; i < 4; i++)
    {
        float produs = directie.scalar(normale[i]);
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
int taierePlan(vec2 intrari[2], int nrIntrari, vec2 iesiri[2], vec2 punctPlan, vec2 normalaPlan)
{
    int nrIesiri = 0;
    if (nrIntrari < 2)
    return 0; // Trebuie sa avem un segment valid
    
    // Calculam d(P) pentru ambele capete
    float d1 = (intrari[0] - punctPlan).scalar(normalaPlan);
    float d2 = (intrari[1] - punctPlan).scalar(normalaPlan);
    
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
        vec2 J_prim = intrari[0] + (intrari[1] - intrari[0]) * t;
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
    vec2 t = ref.p2 - ref.p1;
    float lungime = std::sqrt(t.x * t.x + t.y * t.y);
    t.x /= lungime;
    t.y /= lungime;
    
    // Normala la suprafata de contact n
    vec2 n = ref.n;
    
    // Vectorii pentru prelucrarea in cascada a laturii incidente J1J2
    vec2 intrari[2] = {inc.p1, inc.p2};
    vec2 iesiri[2];
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
    vec2 minus_t(-t.x, -t.y);
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
        float adancime = (intrari[i] - ref.p1).scalar(n);
        
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
    vec2 normala_A = inter.normala;
    vec2 normala_B = (-1)*inter.normala;
    
    PuncteContact contacte;
    contacte.nrPuncte = 0;
    
    // ==============================================================
    // 1. GENERARE MANIFOLD (Puncte de contact)
    // ==============================================================
    if(tipA == DREPTUNGHI && tipB == DREPTUNGHI)
    {
        Latura fataA = gasesteFataSuport(S, corpA, normala_A);
        Latura fataB = gasesteFataSuport(S, corpB, normala_B);
        
        float dotA = fataA.n.scalar(normala_A);
        float dotB = fataB.n.scalar(normala_B);
        
        Latura laturaReferinta, laturaIncidenta;
        if (dotB > dotA + 0.001f) {
            laturaReferinta = fataB;
            laturaIncidenta = fataA;
        } else {
            laturaReferinta = fataA;
            laturaIncidenta = fataB;
        }
        contacte = extrageManifold(laturaReferinta, laturaIncidenta);
    }
    else if (tipA == CERC && tipB == CERC) 
    {
        contacte.nrPuncte = 1;
        contacte.adancimi[0] = inter.adancimee;
        contacte.puncte[0] = A.pozitie + normala_A * A.collider.dimensiune1;
    }
    else 
    {
        contacte.nrPuncte = 1;
        contacte.adancimi[0] = inter.adancimee;
        if (tipA == CERC) {
            contacte.puncte[0] = A.pozitie + normala_A * A.collider.dimensiune1;
        } else {
            contacte.puncte[0] = B.pozitie - normala_A * B.collider.dimensiune1;
        }
    }
    
    if (contacte.nrPuncte == 0) return;
    
    // ==============================================================
    // 2. PREGATIRE CONSTANTE FIZICE
    // ==============================================================
    float invM_A = (A.M > 1e10f) ? 0.0f : 1.0f / A.M;
    float invM_B = (B.M > 1e10f) ? 0.0f : 1.0f / B.M;
    float invI_A = (A.J > 1e10f) ? 0.0f : 1.0f / A.J;
    float invI_B = (B.J > 1e10f) ? 0.0f : 1.0f / B.J;
    
    if (invM_A + invM_B == 0.0f) return;
    
    float k = A.material.restituire * B.material.restituire;
    float mu_s = std::sqrt(A.material.frecareStatica * B.material.frecareStatica);
    float mu_d = std::sqrt(A.material.frecareDinamica * B.material.frecareDinamica);
    
    vec2 n = inter.normala;
    vec2 t(-n.y, n.x); // Tangenta la suprafata de contact
    
    // ==============================================================
    // 3. REZOLVARE IMPULSURI (Pentru fiecare punct de contact)
    // ==============================================================
    for (int i = 0; i < contacte.nrPuncte; i++)
    {
        vec2 rA = contacte.puncte[i] - A.pozitie;
        vec2 rB = contacte.puncte[i] - B.pozitie;
        
        // --- A. IMPULS NORMAL (P) ---
        float rAn = rA.x * n.y - rA.y * n.x;
        float rBn = rB.x * n.y - rB.y * n.x;
        
        float v_rel_n = (B.viteza.x + (-B.omega * rB.y) - (A.viteza.x + (-A.omega * rA.y))) * n.x +
        (B.viteza.y + ( B.omega * rB.x) - (A.viteza.y + ( A.omega * rA.x))) * n.y;
        
        float numitor_n = invM_A + invM_B + (rAn * rAn) * invI_A + (rBn * rBn) * invI_B;
        float v_bias = 20.0f * std::max(contacte.adancimi[i] - 0.005f, 0.0f);
        
        float P = (-(1.0f + k) * v_rel_n + v_bias) / (numitor_n * (float)contacte.nrPuncte);
        if (P < 0.0f) P = 0.0f;
        
        // Aplicare viteze (Normal)
        A.viteza = A.viteza - n * (P * invM_A);
        A.omega  -= P * rAn * invI_A;
        B.viteza = B.viteza + n * (P * invM_B);
        B.omega  += P * rBn * invI_B;
        
        // --- B. IMPULS TANGENTIAL (Frecare) ---
        float rAt = rA.x * t.y - rA.y * t.x;
        float rBt = rB.x * t.y - rB.y * t.x;
        
        float v_rel_t = (B.viteza.x + (-B.omega * rB.y) - (A.viteza.x + (-A.omega * rA.y))) * t.x +
        (B.viteza.y + ( B.omega * rB.x) - (A.viteza.y + ( A.omega * rA.x))) * t.y;
        
        float numitor_t = invM_A + invM_B + (rAt * rAt) * invI_A + (rBt * rBt) * invI_B;
        
        float Jt_teoretic = -v_rel_t / (numitor_t * (float)contacte.nrPuncte);
        float Jt_final = (std::abs(Jt_teoretic) <= P * mu_s) ? Jt_teoretic : (Jt_teoretic > 0.0f ? 1.0f : -1.0f) * P * mu_d;
        
        // Aplicare viteze (Frecare)
        A.viteza = A.viteza - t * (Jt_final * invM_A);
        A.omega  -= Jt_final * rAt * invI_A;
        B.viteza = B.viteza + t * (Jt_final * invM_B);
        B.omega  += Jt_final * rBt * invI_B;

        float scala = 60.0f; 
        vec2 Fn_viz = n * (P * scala);
        vec2 Ft_viz = t * (Jt_final * scala);

        if (invM_A > 0.0f) {
            A.forte_desen.forte.push_back({FORTA_IMPACT_NORMAL, Fn_viz * (-1.0f), contacte.puncte[i]});
            A.forte_desen.forte.push_back({FORTA_IMPACT_FRECARE, Ft_viz * (-1.0f), contacte.puncte[i]});
        }
        if (invM_B > 0.0f) {
            B.forte_desen.forte.push_back({FORTA_IMPACT_NORMAL, Fn_viz, contacte.puncte[i]});
            B.forte_desen.forte.push_back({FORTA_IMPACT_FRECARE, Ft_viz, contacte.puncte[i]});
        }
        
    }
    
    // --- C. FRECARE ROSTOGOLIRE (Optional) ---
    float s_s = std::sqrt(A.material.frecareRostogolireStatica * B.material.frecareRostogolireStatica);
    float s_d = std::sqrt(A.material.frecareRostogolireDinamica * B.material.frecareRostogolireDinamica);
    float invI_suma = invI_A + invI_B;
    if(invI_suma > 0.0001f){
        float d_omega = B.omega - A.omega;
        float P_ung_teoretic = -d_omega / (invI_suma * (float)contacte.nrPuncte);
        // Folosim P (impulsul normal total) din ultima iteratie ca aproximare
        float P_mediu = (-(1.0f + k) * ( (B.viteza - A.viteza).scalar(n) )) / invM_A + invM_B; // Foarte simplificat
        float P_ung_final = (std::abs(P_ung_teoretic) <= std::max(P_mediu, 0.1f) * s_s) ? P_ung_teoretic : (P_ung_teoretic > 0.0f ? 1.0f : -1.0f) * std::max(P_mediu, 0.1f) * s_d;
        
        A.omega -= P_ung_final * invI_A;
        B.omega += P_ung_final * invI_B;
    }
    
    

    S.incarcaStare();
}

void percutiiDeLegatura(sistem &S){                   //rezolva sistemul (J * A^-1 * J^T) Lambda_perc = - J * q*punct
    
    S.seteazaJacobian();
    
    if(S.p == 0) return;
    
    S.LambdaPerc =  matrice(S.p, 1);
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
        q(i*3 + 0 ,0) = S.corpuri[i].viteza.x;
        q(i*3 + 1 ,0) = S.corpuri[i].viteza.y;
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
            suma += L(j,i)* S.LambdaPerc(j,0);          // L(j,i) este L^T(i,j)
        } 
        float valoare = (y(i,0) - suma) / L(i,i);
        if (valoare > percutie_maxima) valoare = percutie_maxima;
        else if (valoare < -percutie_maxima) valoare = -percutie_maxima;
        
        S.LambdaPerc(i,0) = valoare;
    }
    
    matrice Delta_q = S.A_inv * J_T *S.LambdaPerc;
    
    for(int i = 0; i < nr_corpuri; i++) {
        if (S.corpuri[i].M > 1e10f) continue; // Corpurile fixe nu se misca
        
        float dv_x = Delta_q(i * 3 + 0, 0);
        float dv_y = Delta_q(i * 3 + 1, 0);
        float d_omega = Delta_q(i * 3 + 2, 0);
        
        if(dv_x > 100.0f) dv_x = 100.0f; else if(dv_x < -100.0f) dv_x = -100.0f;
        if(dv_y > 100.0f) dv_y = 100.0f; else if(dv_y < -100.0f) dv_y = -100.0f;
        if(d_omega > 50.0f) d_omega = 50.0f; else if(d_omega < -50.0f) d_omega = -50.0f;
        
        S.corpuri[i].viteza.x   += dv_x;
        S.corpuri[i].viteza.y   += dv_y;
        S.corpuri[i].omega += d_omega;
    }
}

void adaugaFortePercutanteVizuale(sistem &S){
    if (S.p == 0) return;

    int index_forta = 0;
    // Transformam impulsul P intr-o forta "aparenta". 
    // Daca sagetile sunt prea mari/mici, ajusteaza aceasta scala.
    float scala = 60.0f; 

    for(int i = 0; i < S.legaturi.size(); i++) {

        if (S.legaturi[i]->activ == 0) continue;

        int nr_ecuatii = S.legaturi[i]->getNumarEcuatii();

        if(nr_ecuatii >= 2){
        
        float Px = S.LambdaPerc(index_forta + 0, 0);
        float Py = S.LambdaPerc(index_forta + 1, 0);
        
        vec2 forta_aparenta = vec2(Px, Py) * scala;

        // Desenam sageata doar daca socul a fost semnificativ (filtram zgomotul de 0.0001)
        if (forta_aparenta.modul() > 1.0f) { 
            vec2 punct_global = S.legaturi[i]->getPozitie(S.corpuri);
            int idA = S.legaturi[i]->contorCorpA;
            int idB = S.legaturi[i]->contorCorpB;

            if (S.corpuri[idA].M < 1e10f) {
                S.corpuri[idA].forte_desen.forte.push_back({FORTA_IMPACT_NORMAL, forta_aparenta, punct_global});
            }
            if (S.corpuri[idB].M < 1e10f) {
                S.corpuri[idB].forte_desen.forte.push_back({FORTA_IMPACT_NORMAL, forta_aparenta * (-1.0f), punct_global});
            }
        }

    }
        index_forta += nr_ecuatii;
    }
}

void verificarCiocniri(sistem &S, editor &E)
{
    bool aFostCiocnire = 0;
    
    for (int i = 0; i < S.corpuri.size(); i++)
    {
        if (S.corpuri[i].activ) {
            S.corpuri[i].seteazaBoundingBox();
        }
    }
    
    for (int i = 0; i < S.corpuri.size(); i++)
    {
        if (!S.corpuri[i].activ) continue;
        
        for (int j = i + 1; j < S.corpuri.size(); j++)
        {
            if (!S.corpuri[j].activ) continue;
            
            if (S.corpuri[i].M > 1e10f && S.corpuri[j].M > 1e10f) // daca luam doi pereti, nu incercam sa calculam ciocnirea dintre ei
            continue;
            if (S.corpuri[i].collider.obiectVirtual || S.corpuri[j].collider.obiectVirtual) 
            continue;
            
            if (intersectareScaraLarga(S, i, j)) {
                intersectie inter = intersectareScaraMica(S, i, j);
                if (inter.seLovesc) {
                    ciocnire(S, i, j, inter);
                    aFostCiocnire = true;
                }
            }
        }
    }

    if(aFostCiocnire){
        percutiiDeLegatura(S);
        adaugaFortePercutanteVizuale(S);
    }
}