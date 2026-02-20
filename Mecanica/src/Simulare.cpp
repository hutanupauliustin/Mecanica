#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <iostream>


class Matrice {
    public:
    int coloane;
    int linii;
    float *valori;
    
    //Constructor
    
    Matrice(){
        this->linii = 0;
        this->coloane = 0;
        this->valori = NULL;
    }
    
    Matrice(int n,int m){
        this->linii = n;
        this->coloane = m;
        this->valori = new float[n*m]();
    }
    
    Matrice(char type,int n,int m){
        
        switch(type){
            case 'I':
            case 'i':
            case '1':   
                if(n != m ){
                    errno = 1;
                    this->linii = 0;
                    this->coloane = 0;
                    this->valori = NULL;
                }
                else{
                    this->linii = n;
                    this->coloane = m;
                    this->valori = new float[n*m];
                    for( int i = 0; i < n; i++)
                        for( int j = 0; j < m; j++)
                            (*this->at(i,j)) = ((i == j) ? 1 : 0);
                }
                break;
            case 'O':
            case 'o':
            case '0':
                this->linii = n;
                this->coloane = m;
                this->valori = new float[n*m];
                for( int i = 0; i < n; i++)
                    for( int j = 0; j < m; j++)
                        (*this->at(i,j))= 0;
                break;
            default:
                errno = 1;
                this->linii = 0;
                this->coloane = 0;
                this->valori = NULL;
        }
        
    }

    Matrice(const Matrice &other){

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
    
    ~Matrice(){
        delete[] this->valori;
        this->valori = NULL;
    }

    //Metode

    float *at(int i, int j) const {                        //returneaza pointer la adresa de memorie unde se afla 
        return this->valori+i*this->coloane + j;    //coloana j si linia i din tabelul de valori
    }
    
    Matrice transpose() const{
        
        Matrice T(this->coloane,this->linii);
        
        for(int i = 0 ; i < this->linii; i++)
        for(int j = 0 ; j < this->coloane; j++){
            *T.at(j,i) = *(this->at(i,j));
        }
        
        return T;
    }
    
    Matrice inverse() const{                   //metoda lui Gauss folosind operatii pe linii si coloane
        
        if(this->linii != this->coloane){
            errno = 1;
            Matrice O;
            return O;
        }
        
        int n = this->linii;
        Matrice B('I',n,n);
        Matrice A = *this;
        
        for(int k = 0; k < n; k++){
            float pivot  = *(A.at(k,k));
            for (int j = 0; j < n; j++) {
                *(A.at(k, j)) /= pivot;
                *(B.at(k, j)) /= pivot;
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
    
    void printMatrice(){
        for(int i = 0; i < this->linii; i++){
            for(int j = 0; j < this->coloane; j++)
                fprintf(stdout,"%8.4f ",(*this->at(i,j)));
            fprintf(stdout,"\n");
        }
    }

    //Operatori

        Matrice& operator= (const Matrice &B){

            if(this == &B)
                return *this;

            this->linii = B.linii;
            this->coloane = B.coloane;

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
        
        Matrice operator+ (const Matrice &B) const{
            
            if((this->linii != B.linii) || (this->coloane != B.coloane)){
                errno = 1;
                Matrice O;
                return O;
            }
            
            int n = this->linii;
            int m = this->coloane;
            
            Matrice S(n,m);
            
            for(int i = 0; i < n; i++)
            for(int j = 0; j < m; j++){
                *(S.at(i,j)) = *(this->at(i,j)) + *(B.at(i,j)); 
            }
            
            return S;
            
        }

        Matrice operator* (const Matrice &B) const{
            
            if( this->coloane != B.linii ){
                errno = 1;
                Matrice O;
                return O;
            }
            
            int n = this->linii;
            int m = B.coloane;
            int p = this->coloane;
            
            Matrice P(n,m);
            
            for(int i =  0 ; i < n; i++)
            for(int j = 0; j < m; j++){
                float s = 0;
                for(int k = 0; k < p; k++)
                s += (*(this->at(i,k)))*(*(B.at(k,j)));
                *(P.at(i,j)) = s;
                
            }
            
            return P;
            
        }

        Matrice operator* (const float x) const{

            Matrice R;
            R = *this;

            for( int i = 0; i < this->linii; i++)
                for(int j = 0; j < this->coloane; j++)
                    *(R.at(i,j)) = (*this->at(i,j)) * x;

            return R;
        }
        
        Matrice operator^ (char *pow) {
            
            if(*pow == 'T' || *pow == 't'){
                return this->transpose();
            }else{
            

            char inverseFlag;
            
            if(pow[0] == '-')
            inverseFlag = 1;
            else
            inverseFlag = 0;
            
            Matrice R('I',this->linii,this->coloane);
            
            int power = atoi(pow + (inverseFlag == 1 ? 1 : 0));
            
            for(int i = 0; i < power; i++){
                R = R * (*this);
            }
            
            if(inverseFlag){
                R = R.inverse();
            }

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

Matrice operator*(const float x, const Matrice &M){
    return M*x;
}

Matrice f(const Matrice &x, float t){
    int n = x.linii / 2;

    Matrice q(n,1) , q_punct(n,1);
    
    for(int i = 0; i < n; i++){
        q(i,0) = x(i,0);
        q_punct(i,0) = x(i+n,0);
    }

    Matrice M('I',n,n);

    Matrice C(n,n);

    Matrice G(n,1);

    Matrice Tau(n,1);

    Matrice M_inversa = M.inverse();

    Matrice q_ppunct(n,1);
    
    q_ppunct = M_inversa * (Tau + (C*q_punct*(-1.0f)) + (G*(-1.0f)));

    Matrice f_rezultat(2*n,1);

    for(int i = 0; i < n; i++){
        f_rezultat(i,0) = q_punct(i,0);
        f_rezultat(i+n,0) = q_ppunct(i,0);
    }

    return f_rezultat;
}

Matrice RK4(const Matrice &x, float dt, float t){

    int n = x.linii / 2;

    Matrice k1(2*n,1),k2(2*n,1),k3(2*n,1),k4(2*n,1), x_nou(2*n,1);

    k1 = dt * f(x,t);
    k2 = dt * f(x + 0.5f*k1,t + 0.5f*dt);
    k3 = dt * f(x + 0.5f*k2,t + 0.5f*dt);
    k4 = dt * f(x + k3, t + dt);

    x_nou = x + (1.0f / 6.0f) * (k1 + 2.0f*k2 + 2.0f*k3 + k4);

    return x_nou;
}

int main(){

    
    
}