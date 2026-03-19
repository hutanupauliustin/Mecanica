#include <cmath>
#include "legatura.h"


    legatura::legatura(int a, int b) : contorCorpA(a), contorCorpB(b) {}

    articulatie::articulatie(int a, int b, float lxa, float lya, float lxb, float lyb)
        : legatura(a, b), l_xA(lxa), l_yA(lya), l_xB(lxb), l_yB(lyb) {}

    int articulatie::getNumarEcuatii() const 
    {
        return 2;
    }

    float articulatie::getAbscisa(matrice &stare) {
        
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        return xA + this->l_xA * cos(phiA) - this->l_yA * sin(phiA);
    }

    float articulatie::getOrdonata(matrice &stare) {
        
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        return yA + this->l_xA * sin(phiA) + this->l_yA * cos(phiA);
    }

    void articulatie::getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi) {
        type = 1;
        widht = 0.5f;
        height = 0.5f;
        phi = 0.0f;
    }


    void articulatie::calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare)  {
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        float xB = stare(idxB + 0, 0);
        float yB = stare(idxB + 1, 0);  
        float phiB = stare(idxB + 2, 0);

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);

        F(rand_start, 0) = xA + this->l_xA * cosA - this->l_yA * sinA - (xB + this->l_xB *cosB - this->l_yB * sinB);
        F(rand_start + 1, 0) = yA + this->l_xA * sinA + this->l_yA * cosA - (yB + this->l_xB * sinB + this->l_yB * cosB);

    }

    void articulatie::calculeazaConstrangereDerivate(matrice &Fpunct, int rand_start, const matrice &stare, int n)  {
        int offsetViteze = 3 * n; 

        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;

        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        float vxA = stare(idxA +  offsetViteze, 0);
        float vyA = stare(idxA + 1 + offsetViteze, 0);
        float phiPunctA = stare(idxA + 2 + offsetViteze, 0);

        float xB = stare(idxB + 0, 0);
        float yB = stare(idxB + 1, 0);  
        float phiB = stare(idxB + 2, 0);

        float vxB = stare(idxB + offsetViteze, 0);
        float vyB = stare(idxB + 1 + offsetViteze, 0);
        float phiPunctB = stare(idxB + 2 + offsetViteze, 0);

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);

        Fpunct(rand_start, 0) = vxA - phiPunctA* this->l_xA * sinA - phiPunctA*this->l_yA * cosA - (vxB - phiPunctB* this->l_xB *sinB - phiPunctB*this->l_yB * cosB);
        Fpunct(rand_start + 1, 0) = vyA + phiPunctA*this->l_xA * cosA - phiPunctA*this->l_yA * sinA - (vyB + phiPunctB*this->l_xB * cosB - phiPunctB*this->l_yB * sinB);

    }

    void articulatie::calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) 
{
    int idxA = contorCorpA * 3;
    int idxB = contorCorpB * 3;
    
    float phiA = stare(idxA + 2, 0);
    float phiB = stare(idxB + 2, 0);

    float sinA = sin(phiA);
    float cosA = cos(phiA);
    float sinB = sin(phiB);
    float cosB = cos(phiB);

    // randul lui f_p+1 -- constrangerea pe OX
    J_F(rand_start, idxA + 0) = 1.0f;                       // coloana x_A
    J_F(rand_start, idxA + 1) = 0.0f;                       // coloana y_A
    J_F(rand_start, idxA + 2) = -l_xA * sinA - l_yA * cosA; // coloana phi_A

    J_F(rand_start, idxB + 0) = -1.0f;                      // coloana x_B
    J_F(rand_start, idxB + 1) = 0.0f;                       // coloana y_B
    J_F(rand_start, idxB + 2) = l_xB * sinB + l_yB * cosB;  // coloana phi_B

    // randul lui f_p+2 -- constrangerea pe OY
    J_F(rand_start + 1, idxA + 0) = 0.0f;                      // coloana x_A
    J_F(rand_start + 1, idxA + 1) = 1.0f;                      // coloana y_A
    J_F(rand_start + 1, idxA + 2) = l_xA * cosA - l_yA * sinA; // coloana phi_A

    J_F(rand_start + 1, idxB + 0) = 0.0f;                      // coloana x_B
    J_F(rand_start + 1, idxB + 1) = -1.0f;                     // coloana y_B
    J_F(rand_start + 1, idxB + 2) = -l_xB * cosB + l_yB * sinB; // coloana phi_B
}

    void articulatie::calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) {
        
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        // Offsetul pentru viteze este 3*n (cate 3 coordonate per corp)
        int offsetViteze = 3 * n; 

        float phiA = stare(idxA + 2, 0);
        float phiB = stare(idxB + 2, 0);
        
        float phiPunctA = stare(idxA + 2 + offsetViteze, 0);
        float phiPunctB = stare(idxB + 2 + offsetViteze, 0);

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);

        // Termenii -J_dot * q_dot
        
        // Componenta X
        float termA_X = -l_xA * (phiPunctA * phiPunctA) * cosA + l_yA * (phiPunctA * phiPunctA) * sinA;
        float termB_X = l_xB * (phiPunctB * phiPunctB) * cosB - l_yB * (phiPunctB * phiPunctB) * sinB;
        
        JdotQ(rand_start, 0) = termA_X + termB_X; 
        
        // Componenta Y
        float termA_Y = -l_xA * (phiPunctA * phiPunctA) * sinA - l_yA * (phiPunctA * phiPunctA) * cosA;
        float termB_Y = l_xB * (phiPunctB * phiPunctB) * sinB + l_yB * (phiPunctB * phiPunctB) * cosB;
        
        JdotQ(rand_start + 1, 0) = termA_Y + termB_Y;
    }  

    // --- FACTORY METHOD ---
    // Permite definirea articulatiei folosind coordonate GLOBALE (mult mai usor de vizualizat)
    articulatie* articulatie::Creaza(rigid& A, rigid& B, float globalX, float globalY) {
        // Calculam vectorul de la centrul corpului la punctul de legatura (in coordonate globale)
        float dxA = globalX - A.x;
        float dyA = globalY - A.y;
        
        float dxB = globalX - B.x;
        float dyB = globalY - B.y;

        float l_xA = dxA * cos(A.phi) + dyA * sin(A.phi);
        float l_yA = -dxA * sin(A.phi) + dyA * cos(A.phi);

        float l_xB = dxB * cos(B.phi) + dyB * sin(B.phi);
        float l_yB = -dxB * sin(B.phi) + dyB * cos(B.phi);

        return new articulatie(A.index, B.index, l_xA, l_yA, l_xB, l_yB);
    }

    incastrare::incastrare(int a, int b, float lxa, float lya, float lxb, float lyb, float unghiInitial)
        : legatura(a, b), l_xA(lxa), l_yA(lya), l_xB(lxb), l_yB(lyb), phi_0(unghiInitial) {}

    int incastrare::getNumarEcuatii() const 
    {
        return 3;
    }

    float incastrare::getAbscisa(matrice &stare) {
        
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        return xA + this->l_xA * cos(phiA) - this->l_yA * sin(phiA);
    }

    float incastrare::getOrdonata(matrice &stare) {
        
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        return yA + this->l_xA * sin(phiA) + this->l_yA * cos(phiA);
    }

         void incastrare::getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi) {
        int idxA = contorCorpA * 3;
        float phiA = stare(idxA + 2, 0);

        type = 2;
        widht = 0.5f;
        height = 0.5f;
        phi = phiA; 
        }


    void incastrare::calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare)  {
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        float xB = stare(idxB + 0, 0);
        float yB = stare(idxB + 1, 0);  
        float phiB = stare(idxB + 2, 0);

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);

        F(rand_start, 0) = xA + this->l_xA * cosA - this->l_yA * sinA - (xB + this->l_xB *cosB - this->l_yB * sinB);
        F(rand_start + 1, 0) = yA + this->l_xA * sinA + this->l_yA * cosA - (yB + this->l_xB * sinB + this->l_yB * cosB);
        F(rand_start + 2, 0) = phiA - phiB - phi_0;


    }

    void incastrare::calculeazaConstrangereDerivate(matrice &Fpunct, int rand_start, const matrice &stare, int n)  {
        int offsetViteze = 3 * n; 

        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;

        float xA = stare(idxA + 0, 0);
        float yA = stare(idxA + 1, 0);
        float phiA = stare(idxA + 2, 0);

        float vxA = stare(idxA +  offsetViteze, 0);
        float vyA = stare(idxA + 1 + offsetViteze, 0);
        float phiPunctA = stare(idxA + 2 + offsetViteze, 0);

        float xB = stare(idxB + 0, 0);
        float yB = stare(idxB + 1, 0);  
        float phiB = stare(idxB + 2, 0);

        float vxB = stare(idxB + offsetViteze, 0);
        float vyB = stare(idxB + 1 + offsetViteze, 0);
        float phiPunctB = stare(idxB + 2 + offsetViteze, 0);

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);

        Fpunct(rand_start, 0) = vxA - phiPunctA* this->l_xA * sinA - phiPunctA*this->l_yA * cosA - (vxB - phiPunctB* this->l_xB *sinB - phiPunctB*this->l_yB * cosB);
        Fpunct(rand_start + 1, 0) = vyA + phiPunctA*this->l_xA * cosA - phiPunctA*this->l_yA * sinA - (vyB + phiPunctB*this->l_xB * cosB - phiPunctB*this->l_yB * sinB);
        Fpunct(rand_start + 2, 0) = phiPunctA - phiPunctB;

    }

    void incastrare::calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) 
    {
        int indexA = contorCorpA * 3;
        int indexB = contorCorpB * 3;
        float phiA = stare(indexA + 2, 0);
        float phiB = stare(indexB + 2, 0);

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);

        // randul lui f_p+1 -- constrangerea pe OX

        J_F(rand_start, indexA + 0) = 1.0f;                       // indexA + 0 este x_A
        J_F(rand_start, indexA + 1) = 0.0f;                       // indexA + 1 este y_A
        J_F(rand_start, indexA + 2) = -l_xA * sinA - l_yA * cosA; // indexA + 2 este phi_A

        J_F(rand_start, indexB + 0) = -1.0f;
        J_F(rand_start, indexB + 1) = 0.0f;
        J_F(rand_start, indexB + 2) = l_xB * sinB + l_yB * cosB;

        // randul lui f_p+2 -- constrangerea pe OY

        J_F(rand_start + 1, indexA + 0) = 0.0f;                      // indexA + 0 este x_A
        J_F(rand_start + 1, indexA + 1) = 1.0f;                      // indexA + 1 este y_A
        J_F(rand_start + 1, indexA + 2) = l_xA * cosA - l_yA * sinA; // indexA + 2 este phi_A

        J_F(rand_start + 1, indexB + 0) = 0.0f;
        J_F(rand_start + 1, indexB + 1) = -1.0f;
        J_F(rand_start + 1, indexB + 2) = -l_xB * cosB + l_yB * sinB;

        // randul lui f_p+3 -- constrangerea fata de Phi

        J_F(rand_start + 2, indexA + 0) = 0.0f;  // indexA + 0 este x_A
        J_F(rand_start + 2, indexA + 1) = 0.0f;  // indexA + 1 este y_A
        J_F(rand_start + 2, indexA + 2) = 1.0f; // indexA + 2 este phi_A

        J_F(rand_start + 2, indexB + 0) = 0.0f;
        J_F(rand_start + 2, indexB + 1) = 0.0f;
        J_F(rand_start + 2, indexB + 2) = -1.0f;
    }

   void incastrare::calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n)  {
        int idxA = contorCorpA * 3;
        int idxB = contorCorpB * 3;
        int offsetViteze = 3 * n; 

        float phiA = stare(idxA + 2, 0);
        float phiB = stare(idxB + 2, 0);
        
        float phiPunctA = stare(idxA + 2 + offsetViteze, 0);
        float phiPunctB = stare(idxB + 2 + offsetViteze, 0);

        float sinA = sin(phiA); float cosA = cos(phiA);
        float sinB = sin(phiB); float cosB = cos(phiB);

        // Componenta X (Identic ca la articulatie)
        float termA_X = -l_xA * (phiPunctA * phiPunctA) * cosA + l_yA * (phiPunctA * phiPunctA) * sinA;
        float termB_X = l_xB * (phiPunctB * phiPunctB) * cosB - l_yB * (phiPunctB * phiPunctB) * sinB;
        JdotQ(rand_start, 0) = termA_X + termB_X; 
        
        // Componenta Y (Identic ca la articulatie)
        float termA_Y = -l_xA * (phiPunctA * phiPunctA) * sinA - l_yA * (phiPunctA * phiPunctA) * cosA;
        float termB_Y = l_xB * (phiPunctB * phiPunctB) * sinB + l_yB * (phiPunctB * phiPunctB) * cosB;
        JdotQ(rand_start + 1, 0) = termA_Y + termB_Y;

        // Componenta pe unghi (E zero)
        JdotQ(rand_start + 2, 0) = 0.0f;
    }

    incastrare* incastrare:: Creaza(rigid& A, rigid& B, float globalX, float globalY) {
        float dxA = globalX - A.x;
        float dyA = globalY - A.y;
        float dxB = globalX - B.x;
        float dyB = globalY - B.y;

        float l_xA = dxA * cos(A.phi) + dyA * sin(A.phi);
        float l_yA = -dxA * sin(A.phi) + dyA * cos(A.phi);

        float l_xB = dxB * cos(B.phi) + dyB * sin(B.phi);
        float l_yB = -dxB * sin(B.phi) + dyB * cos(B.phi);

        // 2. Calculam diferenta de unghi initiala
        float phi0 = A.phi - B.phi;

        return new incastrare(A.index, B.index, l_xA, l_yA, l_xB, l_yB, phi0);
    }