#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <limits>
#include <stdexcept>

struct vec2{
    float x, y;

    vec2(): x(0.0f), y(0.0f) {};
    vec2(float x,float y): x(x), y(y) {};

    vec2 operator+(const vec2 &v) const { return vec2(x + v.x, y + v.y);}
    vec2 operator-(const vec2 &v) const { return vec2(x - v.x, y - v.y);}
    vec2 operator*(const float s) const { return vec2(x*s, y*s);}
    vec2 operator/(const float s) const { return vec2(x/s, y/s);}

    vec2 operator+=(const vec2 &v) {x += v.x; y += v.y; return *this;}
    vec2 operator-=(const vec2 &v) {x -= v.x; y -= v.y; return *this;}

    float scalar(const vec2 &v) const {return x * v.x + y * v.y ;}
    float vectorial(const vec2 &v) const { return x * v.y - y * v.x; }

    float modul() const { return std::sqrt(x * x + y * y); }

};

inline vec2 operator*(float s, const vec2& v) {
    return vec2(v.x * s, v.y * s);
}

class matrice
{
public:
    int coloane;
    int linii;
    float *valori;

    matrice();

    matrice(int n, int m);

    matrice(char type, int n, int m);


    matrice(const matrice &other);
    
    matrice(matrice &&other) noexcept;


    ~matrice();

    matrice &operator=(const matrice &B) ;

    matrice &operator=(matrice &&B) noexcept ;

    float *at(int i, int j) const ;

    float &operator()(int i, int j);
    const float &operator()(int i, int j) const;

    matrice transpose() const ;

    matrice inversaDiagonala() const;

    matrice operator+(const matrice &B) const ;
    matrice operator-(const matrice &B) const ;

    matrice operator-() const ;

    matrice operator*(const matrice &B) const ;

    matrice operator*(const float x) const ;

    matrice operator^(const char *pow) const ;

    void printmatrice() const;
};

matrice operator*(const float x, const matrice &M);