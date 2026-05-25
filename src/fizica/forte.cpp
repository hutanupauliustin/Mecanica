#include "forte.h"

    arc::arc(){
        contorCorpA = 0;
        contorCorpB = 0;
        lungime_0 = 0.0f;
        k = 0.0f;
        d = 0.0f;

    }

    arc::arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0)
        : contorCorpA(a), contorCorpB(b), l_A(lxa,lya), l_B(lxb,lyb), k(k_val), d(d_val), lungime_0(l0) {}


    arc* arc::Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d, float lungime_repaus) {
        // Calculam vectorul de la centrul corpului la punctul de legatura (in coordonate globale)

        vec2 globalA (globalXA, globalYA);
        vec2 globalB (globalXB, globalYB);

        vec2 localA = A.globalToLocal(globalA);
        vec2 localB = B.globalToLocal(globalB);

        // Daca nu se specifica o lungime de repaus (valoare negativa), o calculam ca distanta curenta dintre puncte
        float l0 = lungime_repaus;
        if (l0 < 0.0f) {
            l0 = std::sqrt((globalXB - globalXA) * (globalXB - globalXA) + (globalYB - globalYA) * (globalYB - globalYA));
        }

        return new arc(A.index, B.index, localA.x, localA.y, localB.x, localB.y, constanta_k, constanta_d, l0);
    }
    

    void arc::aplicaForta(std::vector<rigid> &corpuri) {    
        
        vec2 poz1, poz2;
        vec2 viteza1, viteza2;

        rigid& A = corpuri[contorCorpA];
        rigid& B = corpuri[contorCorpB];

        poz1 = A.localToGlobal(l_A);
        poz2 = B.localToGlobal(l_B);

        viteza1 = A.vitezaAbsolutaPunct(l_A);
        viteza2 = B.vitezaAbsolutaPunct(l_B);

        float l = (poz1 - poz2).modul();

        vec2 directie;

        if (l > 0.0001f) {
            directie = (poz2 - poz1) / l;
        } else {
            l = 0.0001f; 
        }

        float viteza_rel = (viteza2 - viteza1).scalar(directie);
        float valoare_forta = -k * (l - lungime_0) - d * viteza_rel;   

        vec2 vector_forta_A = directie * (-valoare_forta);
        vec2 vector_forta_B = directie * (valoare_forta);
        
        A.adauagaForte(-valoare_forta,0.0f,poz1.x, poz1.y,directie.x,directie.y);
        B.adauagaForte( valoare_forta,0.0f,poz2.x, poz2.y,directie.x,directie.y);

        if (A.M < 1e10f) {
            fortaVizuala fA;
            fA.tip = FORTA_ELASTICA;
            fA.valoare = vector_forta_A;
            fA.punct_aplicare = poz1;
            A.forte_desen.forte.push_back(fA);
        }
        if (B.M < 1e10f) {
            fortaVizuala fB;
            fB.tip = FORTA_ELASTICA;
            fB.valoare = vector_forta_B;
            fB.punct_aplicare = poz2;
            B.forte_desen.forte.push_back(fB);
        }

    }

    std::vector<int> arc::getCorpuriAtasate(){

        std::vector<int> corpuriAtasate;
        corpuriAtasate.push_back(this->contorCorpA);
        corpuriAtasate.push_back(this->contorCorpB);
        return corpuriAtasate;
    }
    

    void arc::getGraphics(const matrice &stare, int &type, float &widht, float &height, float &phi, vec2 &pozitieCentru, float &red, float &green, float &blue, float &alpha){
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;

        float phiA = stare(idxA + 2, 0);
        float phiB = stare(idxB + 2, 0);

        float sinA = std::sin(phiA), cosA = std::cos(phiA);
        float sinB = std::sin(phiB), cosB = std::cos(phiB);

        vec2 PA(stare(idxA + 0, 0) + l_A.x * cosA - l_A.y * sinA, stare(idxA + 1, 0) + l_A.x * sinA + l_A.y * cosA);
        vec2 PB(stare(idxB + 0, 0) + l_B.x * cosB - l_B.y * sinB, stare(idxB + 1, 0) + l_B.x * sinB + l_B.y * cosB);

        vec2 diferenta = PB - PA;
        
        pozitieCentru = (PA + PB) * 0.5f;
        phi = std::atan2(diferenta.y, diferenta.x);
        widht = diferenta.modul(); 
        height = this->lungime_0;  

        type = 4; 
        
        red = 1.0f; 
        green = 0.62f; 
        blue = 0.0f; 
        alpha = 1.0f;
}


 motor::motor(){
        contorCorp = 0;
        l.x = 0.0f;
        l.y = 0.0f;
        moment_generat = 0.0f;

}

motor::motor(int corp, float l_X, float l_Y, float moment)
: contorCorp(corp), l(l_X,l_Y), moment_generat(moment) {}


motor* motor::Creaza(rigid&A , float globalX, float globalY, float moment){

        vec2 global_l (globalX, globalY);

        vec2 localA = A.globalToLocal(global_l);
        

        return new  motor(A.index,localA.x,localA.y,moment);
}
    

    void motor::aplicaForta(std::vector<rigid> &corpuri) {    
        
        vec2 poz;

        rigid& A = corpuri[contorCorp];

        poz = A.localToGlobal(l);

        A.adauagaForte(0,moment_generat, 0.0f, 0.0f, 1.0f, 1.0f);

    }

    std::vector<int> motor::getCorpuriAtasate(){

        std::vector<int> corpuriAtasate;
        corpuriAtasate.push_back(this->contorCorp);

        return corpuriAtasate;
    }
    

    void motor::getGraphics(const matrice &stare, int &type, float &widht, float &height, float &phi, vec2 &pozitieCentru, float &red, float &green, float &blue, float &alpha){
        
        int idx = contorCorp * 3;
        float xA   = stare(idx + 0, 0);
        float yA   = stare(idx + 1, 0);
        float phiA = stare(idx + 2, 0);
        float cosA = std::cos(phiA), sinA = std::sin(phiA);

        pozitieCentru.x = xA + l.x * cosA - l.y * sinA;
        pozitieCentru.y = yA + l.x * sinA + l.y * cosA;


        type = 1;
        widht = 0.5f;
        height = 0.5f;
        phi = 0.0f;
        red = 0.902f;
        green = 0.71f;
        blue = 0.902f;
        alpha = 1.0f;
    }