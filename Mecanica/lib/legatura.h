#pragma once
#include "rigid.h"
#include "matrice.h"
#include <cmath>

class legatura
{
public:
    int contorCorpA;
    int contorCorpB;

    legatura();

    legatura(int a, int b) : contorCorpA(a), contorCorpB(b) {}

    virtual ~legatura() = default; // "virtual" ii spune destructorului sa stearga si spatiul utilizat de celelalte clase

    virtual int getNumarEcuatii() const = 0;
    virtual void calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare) = 0;
    virtual void calculeazaConstrangereDerivate(matrice &F, int rand_start, const matrice &stare, int n) = 0;
    virtual void calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) = 0;
    virtual void calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) = 0;
    virtual float getAbscisa(matrice &stare) = 0;
    virtual float getOrdonata(matrice &stare) = 0;
    virtual void getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi) = 0;
};

class articulatie : public legatura
{
private:
    float l_xA, l_yA;
    float l_xB, l_yB;

public:
    articulatie(int a, int b, float lxa, float lya, float lxb, float lyb);

    int getNumarEcuatii() const override;

    float getAbscisa(matrice &stare) override;
    float getOrdonata(matrice &stare) override;

    virtual void getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi) override;


    void calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare) override ;
    void calculeazaConstrangereDerivate(matrice &Fpunct, int rand_start, const matrice &stare, int n) override ;

    void calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) override;

    void calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) override;

    static articulatie* Creaza(rigid& A, rigid& B, float globalX, float globalY);
};

class incastrare : public legatura
{
private:
    float l_xA, l_yA;
    float l_xB, l_yB;
    float phi_0;

public:
    incastrare(int a, int b, float lxa, float lya, float lxb, float lyb, float unghiInitial);

    int getNumarEcuatii() const override;

    float getAbscisa(matrice &stare) override;
    float getOrdonata(matrice &stare) override;

    virtual void getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi);

    void calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare) override;

    void calculeazaConstrangereDerivate(matrice &Fpunct, int rand_start, const matrice &stare, int n) override;

    void calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) override;

   void calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) override ;

    static incastrare* Creaza(rigid& A, rigid& B, float globalX, float globalY) ;
};