#pragma once
#include "rigid.h"
#include "matrice.h"
#include <cmath>

class legatura
{
public:
    
    int contorCorpA;
    int contorCorpB;
    bool activ = false;
    legatura() = default;

    bool selectat = false;
    bool subMouse = false;

    torsor fortaReactiune;

    legatura(int a, int b) : contorCorpA(a), contorCorpB(b) {}

    virtual ~legatura() = default; // "virtual" ii spune destructorului sa stearga si spatiul utilizat de celelalte clase

    virtual int getNumarEcuatii() const = 0;
    virtual void calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare) = 0;
    virtual void calculeazaConstrangereDerivate(matrice &F, int rand_start, const matrice &stare, int n) = 0;
    virtual void calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) = 0;
    virtual void calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) = 0;
    virtual vec2 getPozitie(std::vector<rigid> &corpuri) = 0;
    virtual void getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi) = 0;
};

class articulatie : public legatura
{
private:
    vec2 l_A;         //pozitiile pe A si pe B, in coodronatele lor locale
    vec2 l_B;

public:
    articulatie(int a, int b, float lxa, float lya, float lxb, float lyb);

    int getNumarEcuatii() const override;

    vec2 getPozitie(std::vector<rigid> &corpuri) override;

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
    vec2 l_A;
    vec2 l_B;
    float phi_0;

public:
    incastrare(int a, int b, float lxa, float lya, float lxb, float lyb, float unghiInitial);

    int getNumarEcuatii() const override;

    vec2 getPozitie(std::vector<rigid> &corpuri) override;

    void getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi) override;

    void calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare) override;

    void calculeazaConstrangereDerivate(matrice &Fpunct, int rand_start, const matrice &stare, int n) override;

    void calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) override;

    void calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) override ;

    static incastrare* Creaza(rigid& A, rigid& B, float globalX, float globalY) ;
};

class fir : public legatura
{
private:
    vec2 l_A;
    vec2 l_B;
    float lungime;
    bool tensionat = 0;

public:
    fir(int a, int b, float lxa, float lya, float lxb, float lyb, float unghiInitial);

    int getNumarEcuatii() const override;

    vec2 getPozitie(std::vector<rigid> &corpuri) override;

    virtual void getGraphics(matrice &stare, int &type, float &widht, float &height, float &phi);

    void calculeazaConstrangere(matrice &F, int rand_start, const matrice &stare) override;

    void calculeazaConstrangereDerivate(matrice &Fpunct, int rand_start, const matrice &stare, int n) override;

    void calculeazaJacobian(matrice &J_F, int rand_start, const matrice &stare) override;

    void calculeazaJpunctQpunct(matrice& JdotQ, int rand_start, const matrice &stare, int n) override;

    static fir* Creaza (rigid& A, rigid& B, float globalX_A, float globalY_A, float globalX_B, float globalY_B);
};