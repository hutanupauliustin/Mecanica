#pragma once

class matrice {
    public:
    int coloane;
    int linii;
    float *valori;
    
    //ructor
    
    matrice();
    matrice(int n,int m);
    
    matrice(char type,int n,int m);

    matrice( matrice &other);
    
    ~matrice();
    //Metode

    float *at(int i, int j)  ;
    
    matrice transpose() ;
    
    matrice inverse() ;
    
    void printmatrice();

    //Operatori

        matrice& operator= ( matrice &B);
        matrice operator+ ( matrice &B) ;
        matrice operator* ( matrice &B) ;

        matrice operator* ( float x) ;
        matrice operator^ (char *pow);

        float& operator()(int i, int j);

        float& operator()(int i, int j) ;
};

matrice operator*( float x,  matrice &M);

matrice RK4( matrice &x, float dt, float t);
