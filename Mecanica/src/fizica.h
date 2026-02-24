#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>

class matrice {
    public:
    int coloane;
    int linii;
    float *valori;
    
    //Constructor
    matrice(){
        this->linii = 0;
        this->coloane = 0;
        this->valori = NULL;
    }
    
    matrice(int n,int m){
        this->linii = n;
        this->coloane = m;
        this->valori = new float[n*m]();
    }
    
    matrice(char type,int n,int m){
        switch(type){
            case 'I': case 'i': case '1':   
                if(n != m ){
                    errno = 1; this->linii = 0; this->coloane = 0; this->valori = NULL;
                } else{
                    this->linii = n; this->coloane = m; this->valori = new float[n*m];
                    for( int i = 0; i < n; i++)
                        for( int j = 0; j < m; j++)
                            (*this->at(i,j)) = ((i == j) ? 1 : 0);
                }
                break;
            case 'O': case 'o': case '0':
                this->linii = n; this->coloane = m; this->valori = new float[n*m];
                for( int i = 0; i < n; i++)
                    for( int j = 0; j < m; j++)
                        (*this->at(i,j))= 0;
                break;
            default:
                errno = 1; this->linii = 0; this->coloane = 0; this->valori = NULL;
        }
    }

    matrice(const matrice &other){
        this->linii = other.linii;
        this->coloane = other.coloane;
        if(other.valori != NULL){
            this->valori = new float[this->linii * this->coloane];
            for(int i = 0; i < this->linii * this->coloane; i++)
                this->valori[i] = other.valori[i];
        } else {
            this->valori = NULL;
        }
    }
    
    ~matrice(){
        delete[] this->valori;
        this->valori = NULL;
    }

    //Metode
    float *at(int i, int j) const {                        
        return this->valori+i*this->coloane + j;    
    }
    
    matrice transpose() const{
        matrice T(this->coloane,this->linii);
        for(int i = 0 ; i < this->linii; i++)
        for(int j = 0 ; j < this->coloane; j++){
            *T.at(j,i) = *(this->at(i,j));
        }
        return T;
    }
    
    matrice inverse() const{                  
        if(this->linii != this->coloane){
            errno = 1; matrice O; return O;
        }
        int n = this->linii;
        matrice B('I',n,n);
        matrice A = *this;
        for(int k = 0; k < n; k++){
            float pivot  = *(A.at(k,k));
            for (int j = 0; j < n; j++) {
                *(A.at(k, j)) /= pivot; *(B.at(k, j)) /= pivot;
            }
            for(int i = 0; i < n; i++){
                if( i != k){
                    float x = *(A.at(i,k));
                    for(int j = 0; j < n; j++){
                        *(A.at(i,j)) -= x * (*(A.at(k,j)));
                        *(B.at(i,j)) -= x * (*(B.at(k,j)));
                    }
                }
            }
       }
       return B;
    }
    
    void printmatrice(){
        for(int i = 0; i < this->linii; i++){
            for(int j = 0; j < this->coloane; j++)
                fprintf(stdout,"%8.4f ",(*this->at(i,j)));
            fprintf(stdout,"\n");
        }
    }

    //Operatori
    matrice& operator= (const matrice &B){
        if(this == &B) return *this;
        this->linii = B.linii; this->coloane = B.coloane;
        delete[] this->valori;
        if(B.valori != NULL){
            this->valori = new float[this->linii * this->coloane];
            for(int i = 0; i < this->linii * this->coloane; i++)
                this->valori[i] = B.valori[i];
        } else {
            this->valori = NULL;
        }
        return *this;
    }
        
    matrice operator+ (const matrice &B) const{
        if((this->linii != B.linii) || (this->coloane != B.coloane)){
            errno = 1; matrice O; return O;
        }
        int n = this->linii; int m = this->coloane;
        matrice S(n,m);
        for(int i = 0; i < n; i++)
        for(int j = 0; j < m; j++){
            *(S.at(i,j)) = *(this->at(i,j)) + *(B.at(i,j)); 
        }
        return S;
    }

    matrice operator* (const matrice &B) const{
        if( this->coloane != B.linii ){
            errno = 1; matrice O; return O;
        }
        int n = this->linii; int m = B.coloane; int p = this->coloane;
        matrice P(n,m);
        for(int i =  0 ; i < n; i++)
        for(int j = 0; j < m; j++){
            float s = 0;
            for(int k = 0; k < p; k++)
            s += (*(this->at(i,k)))*(*(B.at(k,j)));
            *(P.at(i,j)) = s;
        }
        return P;
    }

    matrice operator* (const float x) const{
        matrice R; R = *this;
        for( int i = 0; i < this->linii; i++)
            for(int j = 0; j < this->coloane; j++)
                *(R.at(i,j)) = (*this->at(i,j)) * x;
        return R;
    }
        
    matrice operator^ (char *pow) {
        if(*pow == 'T' || *pow == 't'){
            return this->transpose();
        }else{
            char inverseFlag = (pow[0] == '-') ? 1 : 0;
            matrice R('I',this->linii,this->coloane);
            int power = atoi(pow + (inverseFlag == 1 ? 1 : 0));
            for(int i = 0; i < power; i++){
                R = R * (*this);
            }
            if(inverseFlag) R = R.inverse();
            return R;
        }
    }

    float& operator()(int i, int j){
        return *this->at(i,j);
    }

    const float& operator()(int i, int j) const {
        return *this->at(i,j);
    }
};

inline matrice operator*(const float x, const matrice &M){
    return M*x;
}

class particula {
    public:

    float x;
    float y;
    float vx;
    float vy;

    float masa;
    
};
matrice f(const matrice &x, float t);
matrice RK4(const matrice &x, float dt, float t);