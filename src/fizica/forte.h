#pragma once
#include "rigid.h"

class generatorForte{

public:

    bool activ = 1;
    
    struct col{ 
        float x,y,z,w;
    } culoare;


    virtual ~generatorForte() = default;

    virtual void aplicaForta(std::vector<rigid> &corpuri)= 0;

    virtual std::vector<int> getCorpuriAtasate() = 0;

    virtual void getGraphics(const matrice &stare, int &type, float &widht, float &height, float &phi, vec2 &pozitieCentru, float &red, float &green, float &blue, float &alpha) {
        (void)stare;
        (void)widht;
        (void)height;
        (void)phi;
        (void)pozitieCentru;
        red = this->culoare.x;
        green = this->culoare.y;
        blue = this->culoare.z;
        alpha = this->culoare.w;
        type = -1;
    }

        
};


class arc : public generatorForte{

    public:

    int contorCorpA;
    int contorCorpB;

    vec2 l_A;
    vec2 l_B;

    float k,d;  // k--constanta elastica d--constanda de dampening
    float lungime_0;

    arc();

    arc(int a, int b, float lxa, float lya, float lxb, float lyb, float k_val, float d_val, float l0);

    static arc* Creaza(rigid& A, rigid& B, float globalXA, float globalYA, float globalXB, float globalYB, float constanta_k, float constanta_d = 0.0f, float lungime_repaus = -1.0f);
    
    void aplicaForta(std::vector<rigid> &corpuri) override; 

    std::vector<int> getCorpuriAtasate() override;

    void getGraphics(const matrice &stare, int &type, float &widht, float &height, float &phi, vec2 &pozitieCentru, float &red, float &green, float &blue, float &alpha)  override;
    
};