#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <limits>
#include <stdexcept>

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

    matrice inverse() const;

    matrice operator+(const matrice &B) const ;
    matrice operator-(const matrice &B) const ;

    matrice operator-() const ;

    matrice operator*(const matrice &B) const ;

    matrice operator*(const float x) const ;

    matrice operator^(const char *pow) const ;

    void printmatrice() const;
};

inline matrice operator*(const float x, const matrice &M);