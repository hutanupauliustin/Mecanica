#include "sistem.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include "fizica.h"

    float inertie_minima = 0.00001f;
    float viteza_maxima = 500.0f;
    float viteza_unghiulara_maxima = 200.0f;

    sistem::sistem()
    {                                       // A - matricea de inertie
                    // Lambda - vectorul multiplicatorilor lui Lagrange
        p = 0;
        k_d = 0.0f;
        k_s = 0.0f;
        g = 9.81f; // Initializare implicita
        nivelEnergiePotentiala = 0.0f;
        energie = 0.0f;

        stare = matrice(2, 1);

        rigid lume = rigid::Fix(0.0f, 0.0f);
        adaugaCorpuri(lume);
        id_corp_lume = 0;
    }

    sistem::~sistem()
    {
        for (size_t i = 0; i < this->legaturi.size(); i++)
        {
            delete legaturi[i];
        }
        for (size_t i = 0; i < this->surseForte.size(); i++)
        {
            delete surseForte[i];
        }
    }

    void sistem::setareConstantaGravitationala(float grav){
        g = grav;
    }

    void sistem::setareConstanteStabilizare(float spring_constant, float dampening_constant){
        k_s = spring_constant;
        k_d = dampening_constant;
    }


    void sistem::adaugaCorpuri(rigid &r){
        for(size_t i = 0; i < corpuri.size(); i++){
            if(corpuri[i].activ == 0){
                r.index = i;
                r.activ = 1;
                corpuri[i] = r; // Suprascriem corpul vechi
                return;         // Iesim din functie
            }
        }
        // Daca nu am gasit niciun loc liber, adaugam la capat
        r.activ = 1;
        r.index = corpuri.size();
        corpuri.push_back(r);
        actualizeazaMatriceFizica();
    }

    void sistem::adaugaLegaturi(legatura *l)
    {
        for(size_t i = 0; i < legaturi.size(); i++){
            if(legaturi[i]->activ == 0){
                delete legaturi[i];
                l->activ = 1;
                legaturi[i] = l;    
                p += l->getNumarEcuatii();
                return;             
            }
        }
       
        l->activ = 1;
        legaturi.push_back(l);
        p += l->getNumarEcuatii();
        actualizeazaMatriceFizica();
    }

    void sistem::adaugaGeneratorForte(generatorForte *F)
    {
        for(size_t i  = 0; i < surseForte.size(); i++){
            if(surseForte[i]->activ == 0){
                delete surseForte[i];
                F->activ = 1;
                surseForte[i] = F; 
                return;        
            }
        }
        // Daca nu am gasit niciun loc liber, adaugam la capat
       F->activ = 1;
       surseForte.push_back(F);
       actualizeazaMatriceFizica();
    }

    void sistem::eliminaCorp(int index){
        if(index <= 0 || index >= (int) corpuri.size()) return; 
        
        corpuri[index].activ = 0;
        corpuri[index].collider.selectat = 0;
        
        for(size_t i = 0; i < legaturi.size(); i++){
            if(legaturi[i]->activ && (legaturi[i]->contorCorpA == index || legaturi[i]->contorCorpB == index)){
                eliminaLegatura(i);
            }
        }
        
        for(size_t i = 0; i < surseForte.size(); i++){

            if(surseForte[i]->activ == false)
                continue;

            std::vector<int> corpuriAtasate = surseForte[i]->getCorpuriAtasate();

            if(std::find(corpuriAtasate.begin(), corpuriAtasate.end() ,index) != corpuriAtasate.end())
                eliminaGeneratorForte(i);
        }
        
        actualizeazaMatriceFizica();
    }

    void sistem::actualizeazaMatriceFizica() {
        this->p = 0; 
        for (auto* leg : legaturi) {
            if (leg && leg->activ) { // Adaugă verificarea de nullptr aici!
                this->p += leg->getNumarEcuatii();
            }
        }

        incarcaStare();
        seteazaMatriceInertie();
        seteazaJacobian();
        seteazaConstrangeri();
        seteazaForteExterne();
    }

    void sistem::eliminaLegatura(int i){

        int index = i;

        if(index < 0 || index >= (int) legaturi.size()) return;
        if(legaturi[index]->activ == 1) {
            legaturi[index]->activ = 0;
            p -= legaturi[index]->getNumarEcuatii(); // Reducem imediat dimensiunea sistemului matriceal
        }
    }

    void sistem::eliminaGeneratorForte(int i){

        int index = i;

        if(index < 0 || index >= (int) surseForte.size()) return;
        surseForte[index]->activ = 0;
    }


    void sistem::verificaOutOfBounds(std::vector<int> corpuriSelectate, std::vector<int> corpuriSubMouse){
    for(int i = 0; i < (int) this->corpuri.size(); i++ )
        if( std::abs(stare(i*3,0)) > 500 || std::abs(stare(i*3 + 1,0)) > 500 ){
              this->eliminaCorp(i);

            corpuriSelectate.erase(std::remove(corpuriSelectate.begin(), corpuriSelectate.end(), i), corpuriSelectate.end());
            corpuriSubMouse.erase(std::remove(corpuriSubMouse.begin(), corpuriSubMouse.end(), i), corpuriSubMouse.end());
        }
    }

    void sistem::incarcaStare(){

       int nr_corpuri = this->corpuri.size();

        if (stare.linii != 6 * nr_corpuri || (int) stare.coloane != 1) {
            stare = matrice(6 * nr_corpuri, 1);
        }
        
        for (int i = 0; i < nr_corpuri; i++)
        {

            if(this->corpuri[i].activ == 0){
                stare(i * 3, 0) = 0;
                stare(i * 3 + 1, 0) = 0;
                stare(i * 3 + 2, 0) = 0;
                stare(i * 3 + 3 * nr_corpuri, 0) = 0;
                stare(i * 3 + 1 + 3 * nr_corpuri, 0) = 0;
                stare(i * 3 + 2 + 3 * nr_corpuri, 0) = 0;
            }else{
                stare(i * 3, 0) = corpuri[i].pozitie.x;
                stare(i * 3 + 1, 0) = corpuri[i].pozitie.y;
                stare(i * 3 + 2, 0) = corpuri[i].phi ;
                stare(i * 3 + 3 * nr_corpuri, 0) = corpuri[i].viteza.x;
                stare(i * 3 + 1 + 3 * nr_corpuri, 0) = corpuri[i].viteza.y;
                stare(i * 3 + 2 + 3 * nr_corpuri, 0) = corpuri[i].omega;
            }
        }
    }

    void sistem::seteazaStare(){

        int nr_corpuri = this->corpuri.size();
        const float PI = 3.1415926535f;
        const float TWO_PI = 2.0f * PI;

        for (int i = 0; i < (int) nr_corpuri; i++)
        {
            if(this->corpuri[i].activ == 0){
                stare(i * 3, 0) = 0;
                stare(i * 3 + 1, 0) = 0;
                stare(i * 3 + 2, 0) = 0;
                stare(i * 3 + 3 * nr_corpuri, 0) = 0;
                stare(i * 3 + 1 + 3 * nr_corpuri, 0) = 0;
                stare(i * 3 + 2 + 3 * nr_corpuri, 0) = 0;
            }else{
            corpuri[i].pozitie.x = stare(i * 3, 0);
            corpuri[i].pozitie.y = stare(i * 3 + 1, 0);
            
            float phi_redus = std::fmod(stare(i * 3 + 2, 0) + PI, TWO_PI);
            if (phi_redus < 0.0f) phi_redus += TWO_PI;
            phi_redus -= PI;
            
            stare(i * 3 + 2, 0) = phi_redus;
            corpuri[i].phi = phi_redus;
            
            corpuri[i].viteza.x = stare(i * 3 + 3 * nr_corpuri, 0);
            corpuri[i].viteza.y = stare(i * 3 + 1 + 3 * nr_corpuri, 0);
            corpuri[i].omega = stare(i * 3 + 2 + 3 * nr_corpuri, 0);
            }
        }
    }

    void sistem::seteazaJacobian()
    {
        if (p == 0) {
            int nr_corpuri = this->corpuri.size();
            J_F = matrice('0', 0, 3 * nr_corpuri);
            JdotQ = matrice('0', 0, 1);
        return;
        }

        int nr_corpuri = this->corpuri.size();

        if (J_F.linii != p || J_F.coloane != 3 * nr_corpuri){
            J_F = matrice('0', p, 3 * nr_corpuri);
        }
        else{
            for (int i = 0; i < J_F.linii; i++){
                for (int j = 0; j < J_F.coloane; j++){
                    J_F(i, j) = 0.0f;
                }
            }
        }                           //redeclaram jacobianul, daca trebuie modificat numarul de valori
                                    //daca nu, il facem 0 peste tot        
        if(JdotQ.linii != p || JdotQ.coloane != 1){
            JdotQ = matrice('0',p,1);
        } else {
            for(int i = 0; i < p; i++){
                JdotQ(i,0) = 0.0f;
            }
        }

        int rand_constrangere = 0;

        for (size_t i = 0; i < legaturi.size(); i++)
        {
            if(legaturi[i]->activ == 0) continue;

            legaturi[i]->calculeazaJacobian(J_F, rand_constrangere, stare);
            legaturi[i]->calculeazaJpunctQpunct(JdotQ, rand_constrangere, stare, nr_corpuri);
            rand_constrangere += legaturi[i]->getNumarEcuatii();
        }
    }

void sistem::seteazaConstrangeri()
    {
        if (p == 0) {
            int nr_corpuri = this->corpuri.size();
            J_F = matrice('0', 0, 3 * nr_corpuri);
            JdotQ = matrice('0', 0, 1);
        return;
        }

        int nr_corpuri = this->corpuri.size();

        if (F.linii != p || F.coloane != 1)
        {
            F = matrice('0', p, 1);
        }
        else
        {
            for (int i = 0; i < (int) F.linii; i++)
            {
                F(i, 0) = 0.0f;
            }

        }

        if (Fpunct.linii != p || Fpunct.coloane != 1)
        {
            Fpunct = matrice('0', p, 1);
        }
        else
        {
            for (int i = 0; i < (int) Fpunct.linii; i++)
                Fpunct(i, 0) = 0.0f;
        }

        int rand_constrangere = 0;

        for (size_t i = 0; i < legaturi.size(); i++)
        {
            if(legaturi[i]->activ == 0) continue; // Sarim peste cele sterse!

            legaturi[i]->calculeazaConstrangere(F, rand_constrangere, stare);
            legaturi[i]->calculeazaConstrangereDerivate(Fpunct, rand_constrangere, stare, nr_corpuri);
            rand_constrangere += legaturi[i]->getNumarEcuatii();
        }
    }

    void sistem::seteazaMatriceInertie()
    {
        int nr_corpuri = this->corpuri.size();

        if (A.linii != 3 * nr_corpuri || A.coloane != 3 * nr_corpuri) {
            A = matrice('0', 3 * nr_corpuri, 3 * nr_corpuri);
        } else {
            for(int i =0; i < 3*nr_corpuri; i++)
                for(int j = 0; j < 3*nr_corpuri; j++){
                    A(i,j) = 0;
                }
        }

        for (int i = 0; i < nr_corpuri; i++)
        {
            if(this->corpuri[i].activ == 0){
            A(3 * i,     3 * i)     = 0; 
            A(3 * i + 1, 3 * i + 1) = 0; 
            A(3 * i + 2, 3 * i + 2) = 0; 
            }else{
            
            float valoare_masa = corpuri[i].M;
            float valoare_moment_inertie = corpuri[i].J;

            A(3 * i,     3 * i)     = valoare_masa >  inertie_minima ? valoare_masa : inertie_minima;
            A(3 * i + 1, 3 * i + 1) = valoare_masa >  inertie_minima ? valoare_masa : inertie_minima;
            A(3 * i + 2, 3 * i + 2) = valoare_moment_inertie >  inertie_minima ? valoare_moment_inertie : inertie_minima;
            }
        }   
        
        A_inv = A.inversaDiagonala();
    }

    void sistem::seteazaForteExterne()
    {
        int nr_corpuri = this->corpuri.size();

        if (Q.linii != 3 * nr_corpuri || Q.coloane != 1) {
            Q = matrice(3 * nr_corpuri, 1);
        }

        // 1. Initializam fortele (gravitatie, frecare aer)
        for (int i = 0; i < (int) nr_corpuri; i++) {
            corpuri[i].forte_desen.forte.clear();
            corpuri[i].aflaForteProprii(g); 
        }
        // 2. Adaugam generatorii de forte

        for (size_t i = 0; i < surseForte.size(); i++) {
            if(surseForte[i]->activ == 0) continue; 
            surseForte[i]->aplicaForta(this->corpuri);
        }

        for (int i = 0; i < nr_corpuri; i++) {
            corpuri[i].reducereTorsor(); 
        }

        // 3. Incarcam totul in matricea sistemului
        for (int i = 0; i < nr_corpuri; i++) {
            Q(3 * i, 0) = corpuri[i].tau.forta.x;
            Q(3 * i + 1, 0) = corpuri[i].tau.forta.y;
            Q(3 * i + 2, 0) = corpuri[i].tau.moment;
        }
    }

    void sistem::plafonareViteze(){
       for(size_t i = 1; i < corpuri.size(); i++){
            if(std::isnan(corpuri[i].viteza.x) || std::isnan(corpuri[i].viteza.y) || std::isnan(corpuri[i].pozitie.x)) {
                corpuri[i].viteza.x = 0.0f; corpuri[i].viteza.y = 0.0f; corpuri[i].omega = 0.0f;
                corpuri[i].pozitie.x = 0.0f; corpuri[i].pozitie.y = 0.0f; corpuri[i].phi = 0.0f;
            } else {
                if(corpuri[i].viteza.x > viteza_maxima) corpuri[i].viteza.x = viteza_maxima;
                else if(corpuri[i].viteza.x < -viteza_maxima) corpuri[i].viteza.x = -viteza_maxima;
                
                if(corpuri[i].viteza.y > viteza_maxima) corpuri[i].viteza.y = viteza_maxima;
                else if(corpuri[i].viteza.y < -viteza_maxima) corpuri[i].viteza.y = -viteza_maxima;
                
                if(corpuri[i].omega > viteza_unghiulara_maxima) corpuri[i].omega = viteza_unghiulara_maxima;
                else if(corpuri[i].omega < -viteza_unghiulara_maxima) corpuri[i].omega = -viteza_unghiulara_maxima;
            }
        }
    }

    void sistem::step() {
        for(auto& corp : corpuri) {
            corp.forte_desen.reseteaza();
        }

        this->stare = RK4(*this, dt, t);
        this->seteazaStare();       
        this->plafonareViteze();

        verificarCiocniri(*this);  

        this->incarcaStare();
        this->t += dt;
        //S.verificaOutOfBounds(E.corpuriSelectate, E.corpuriSubMouse);   

    }