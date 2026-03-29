#pragma once
#include "rigid.h"

class arc{

    public:

    int contorCorpA;
    int contorCorpB;
    bool activ = 1;

    vec2 l_A;
    vec2 l_B;

    float lungime_0;
    float k,d;  // k--constanta elastica d--constanda de dampening

    arc();

    arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0);
    // Permite definirea arcului folosind coordonate GLOBALE pentru punctele de prindere
    static arc Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d = 0.0f, float lungime_repaus = -1.0f);
    
    void aplicaFortaElastica(rigid &A, rigid &B);
};