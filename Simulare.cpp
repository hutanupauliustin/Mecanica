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
                else
                    this->linii = n;
                    this->coloane = m;
                    this->valori = new float[n*m];
                    for( int i = 0; i < n; i++)
                        for( int j = 0; j < m; j++)
                            (*this->at(i,j)) = ((i == j) ? 1 : 0);
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
        }
        
    }

    Matrice( Matrice &other){

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

    float *at(int i, int j){                        //returneaza pointer la adresa de memorie unde se afla 
        return this->valori+i*this->coloane + j;    //coloana j si linia i din tabelul de valori
    }
    
    Matrice transpose(){
        
        Matrice T(this->coloane,this->linii);
        
        for(int i = 0 ; i < this->linii; i++)
        for(int j = 0 ; j < this->coloane; j++){
            *T.at(j,i) = *(this->at(i,j));
        }
        
        return T;
    }
    
    Matrice inverse(){                   //metoda lui Gauss folosind operatii pe linii si coloane
        
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

        Matrice& operator= (Matrice &B){

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
        
        Matrice operator+ (Matrice B) {
            
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

        Matrice operator* (Matrice B) {
            
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

        Matrice operator* (float x){

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
};

Matrice operator*(float x, Matrice M){
    return M*x;
}

int main(){

    float q[2];
    Matrice x(4,1);  // x[1] = q1 , x[2] = q2, x[3] = q1' , x[4] = q2' 


    
}