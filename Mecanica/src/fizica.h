#pragma once
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <limits>

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
    
    matrice inverse() const{            //deoarece singura matrice pe care trebuie inversata este A, care este diagonala, vom inversa elementele de pe diagonala        

        int n = this->linii;
        matrice B(n,n);

        for(int i = 0; i < n; i++){
            B(i,i) = (*this)(i,i) == std::numeric_limits<float>::max() ? 0 : (*this)(i,i);
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

class rigid{
    public:
        float x,y,phi;
        float v_x,v_y,omega;     //coordonatele centrului de greautea, si unghiul facut de sistemul de referinta propriu fata de cel universal
        float M, J;
        float f_x,f_y,moment;
};

enum{
    ARTICULATIE,
    INCASTRARE
};

class sistem {
    public:
    int n,p;
    rigid *corpuri;
    legatura *legaturi;
    matrice stare;

    matrice Q, J_F, A, Lambda;                  // Q - vectorul fortelor externe
                                                // J_f - Jacobianul legaturilor
    sistem(int nr_corpuri, int nr_legaturi){    // A - matricea de inertie
    n = nr_corpuri;                             // Lambda - vectorul multiplicatorilor lui Lagrange
    p = nr_legaturi;
    corpuri =  new rigid[n];
    legaturi = new legatura[p];
    stare = matrice(6*n,1);
    }

    ~sistem(){
        delete[] corpuri;
    }
    
    void incarcaStare(){
        for(int i = 0; i < n; i++){
            stare(i*3,0)       = corpuri[i].x;
            stare(i*3+1,0)     = corpuri[i].y;
            stare(i*3+2,0)     = corpuri[i].phi;
            stare(i*3 + n,0)   = corpuri[i].v_x;
            stare(i*3+1 + n,0) = corpuri[i].v_y;
            stare(i*3+1 + n,0) = corpuri[i].omega;
            
        }
    }

    void seteazaStare(){
        for(int i = 0; i < n; i++){
           corpuri[i].x = stare(i*3,0);    
           corpuri[i].y = stare(i*3+1,0);
           corpuri[i].phi = stare(i*3+2,0);
           corpuri[i].v_x = stare(i*3 + n,0);
           corpuri[i].v_y = stare(i*3+1 + n,0);
           corpuri[i].omega = stare(i*3+2 + n,0);
        } 
    }

    void seteazaJacobian(){
        for( int i = 0; i <n ; i++){
            corpuri[i].
        }
    }


};

class legatura {
    protected:
        rigid* corpA;
        rigid* corpB;

    public:

        legatura(){
            corpA = NULL;
            corpB = NULL;
        }

        legatura(rigid* a, rigid *b) : corpA(a),corpB(b) {}

        virtual ~legatura() = default;   // ii spune destructorului sa stearga si spatiul utilizat de celelalte clase

        virtual int getNumarEcuatii() const = 0;
        virtual void calculeazaJacobian(matrice& J_F, int rand_start, int indexA, int indexB) = 0;
};

class articulatie : public legatura{
    private: 
    float l_xA, l_yA;
    float l_xB, l_yB;

    public:
    articulatie(rigid* a, rigid* b, float lxa, float lya, float lxb, float lyb) 
        : legatura(a, b), l_xA(lxa), l_yA(lya), l_xB(lxb), l_yB(lyb) {}

    int getNumarEcuatii() const override {
        return 2;
    }

    void calculeazaJacobian(matrice& J_F, int rand_start, int indexA, int indexB) override {
        float phiA = corpA->phi;
        float phiB = corpB->phi;

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);
        
        // randul lui f_p+1 -- constrangerea pe OX

        J_F(rand_start,indexA + 0) = 1.0f; // indexA + 0 este x_A
        J_F(rand_start,indexA + 1) = 0.0f; // indexA + 0 este y_A 
        J_F(rand_start,indexA + 2) = -l_xA * sinA - l_yA * cosA; // indexA + 0 este phi_A     
        
        J_F(rand_start,indexB + 0) = -1.0f; 
        J_F(rand_start,indexB + 1) = 0.0f; 
        J_F(rand_start,indexB + 2) = l_xB * sinB + l_yB * cosB; 

        // randul lui f_p+2 -- constrangerea pe OY

        J_F(rand_start + 1,indexA + 0) = 0.0f; // indexA + 0 este x_A
        J_F(rand_start + 1,indexA + 1) = 1.0f; // indexA + 0 este y_A 
        J_F(rand_start + 1,indexA + 2) = l_xA * cosA - l_yA * sinA; // indexA + 0 este phi_A     
        
        J_F(rand_start + 1,indexB + 0) = 0.0f; 
        J_F(rand_start + 1,indexB + 1) = -1.0f; 
        J_F(rand_start + 1,indexB + 2) = -l_xB * cosB + l_yB * sinB; 

    }
};

class incastrare : public legatura{
    private: 
    float l_xA, l_yA;
    float l_xB, l_yB;
    float phi_0;

    public:
    incastrare(rigid* a, rigid* b, float lxa, float lya, float lxb, float lyb, float unghiInitial) 
        : legatura(a, b), l_xA(lxa), l_yA(lya), l_xB(lxb), l_yB(lyb), phi_0(unghiInitial) {}

    int getNumarEcuatii() const override {
        return 3;
    }

    void calculeazaJacobian(matrice& J_F, int rand_start, int indexA, int indexB) override {
        float phiA = corpA->phi;
        float phiB = corpB->phi;

        float sinA = sin(phiA);
        float cosA = cos(phiA);
        float sinB = sin(phiB);
        float cosB = cos(phiB);
        
        // randul lui f_p+1 -- constrangerea pe OX

        J_F(rand_start,indexA + 0) = 1.0f; // indexA + 0 este x_A
        J_F(rand_start,indexA + 1) = 0.0f; // indexA + 0 este y_A 
        J_F(rand_start,indexA + 2) = -l_xA * sinA - l_yA * cosA; // indexA + 0 este phi_A     
        
        J_F(rand_start,indexB + 0) = -1.0f; 
        J_F(rand_start,indexB + 1) = 0.0f; 
        J_F(rand_start,indexB + 2) = l_xB * sinB + l_yB * cosB; 

        // randul lui f_p+2 -- constrangerea pe OY

        J_F(rand_start + 1,indexA + 0) = 0.0f; // indexA + 0 este x_A
        J_F(rand_start + 1,indexA + 1) = 1.0f; // indexA + 0 este y_A 
        J_F(rand_start + 1,indexA + 2) = l_xA * cosA - l_yA * sinA; // indexA + 0 este phi_A     
        
        J_F(rand_start + 1,indexB + 0) = 0.0f; 
        J_F(rand_start + 1,indexB + 1) = -1.0f; 
        J_F(rand_start + 1,indexB + 2) = -l_xB * cosB + l_yB * sinB; 

        // randul lui f_p+3 -- constrangerea fata de Phi

        J_F(rand_start + 1,indexA + 0) = 0.0f; // indexA + 0 este x_A
        J_F(rand_start + 1,indexA + 1) = 0.0f; // indexA + 0 este y_A 
        J_F(rand_start + 1,indexA + 2) = -1.0f; // indexA + 0 este phi_A     
        
        J_F(rand_start + 1,indexB + 0) = 0.0f; 
        J_F(rand_start + 1,indexB + 1) = 0.0f; 
        J_F(rand_start + 1,indexB + 2) = 1.0f; 

    }
};

matrice f(const matrice &x, float t);
void seteazaForte(sistem &S, float t);
matrice derivate(sistem &S, const matrice &stare_curenta, float t);
matrice RK4(sistem &S, const matrice &x, float dt, float t);