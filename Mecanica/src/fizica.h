#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cmath>

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

    void operator-(matrice &M){
        for(int i = 1; i < M.linii; i++)
            for(int j = 1; j < M.coloane; j++)
                M(i,j) = (-1)* M(i,j);
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
    float fx;
    float fy;
    float masa;
    float sarcina;

    particula(){
        x=0.0f;
        y=0.0f;
        vx=0.0f;
        vy=0.0f;
        fx=0.0f;
        fy=0.0f;
        masa=0.0f;
        sarcina=0.0f;
    }

    particula(float abscisa,float ordonata, float viteza_x, float viteza_y, float masa_particula, float sarcina_particula){
        x = abscisa;
        y = ordonata;
        vx = viteza_x;
        vy = viteza_y;
        masa = masa_particula;
        sarcina = sarcina_particula;
    }

    void greutateProprie(){
        fy += masa * (-9.81f);
    }


};

class sistem {
    public:
    int n;
    particula *v;
    matrice stare;

    sistem(int nr_particule){
    n = nr_particule;
    v =  new particula[n];
    stare = matrice(4*n,1);
    }

    ~sistem(){
        delete[] v;
    }

    float distanta(particula p1, particula p2){
        return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
    }

    matrice vectorPozitie(particula p1, particula p2){
        matrice r(2,1);
        r(0,0) = p2.x - p1.x;
        r(1,0) = p2.y - p1.y;
    }
    
    void incarcaStare(){
        for(int i = 0; i < n; i++){
            stare(i*2,0)       = v[i].x;
            stare(i*2+1,0)     = v[i].y;
            stare(i*2 + n,0)   = v[i].vx;
            stare(i*2+1 + n,0) = v[i].vy;
        }
    }

    void seteazaStare(){
        for(int i = 0; i < n; i++){
           v[i].x = stare(i*2,0);    
           v[i].y = stare(i*2+1,0);
           v[i].vx = stare(i*2 + n,0);
           v[i].vy = stare(i*2+1 + n,0);
        } 
    }
};

matrice f(const matrice &x, float t);
void seteazaForte(sistem &S, float t);
matrice derivate(sistem &S, const matrice &stare_curenta, float t);
matrice RK4(sistem &S, const matrice &x, float dt, float t);