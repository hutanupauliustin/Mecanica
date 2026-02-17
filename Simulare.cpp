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
                    for( int i = 0; j < m; j++)
                            (*this->at(i,j))= 0;
                break;
        }

    }

   
    float *at(int i, int j){                        //returneaza pointer la adresa de memorie unde se afla 
        return this->valori+i*this->coloane + j;    //coloana j si linia i din tabelul de valori
    }

    Matrice transpose(){

        Matrice T(this->coloane,this->linii);

        for(int i = 0 ; i < this->linii; i++)
            for(int j = 0 ; j < this->coloane; j++){
                *T.at(j,i)) = *(this->at(i,j));
            }

        return T;
    }
    
    Matrice inverse(){                   //metoda lui Gauss folosind operatii pe linii si coloane

        if(this->linii != this->coloane){
            errno = 1;
            Matrice O;
            return O;
        }

        Matrice B(I,this->linii,this->coloane);
        Matrice A = *this;

        for(int i = 0; i < this->linii; i++)

            float x = (*A.at(i,i)) / (*A.at());
            for(int j = i; j < this->linii; j++){  // sub diagonalei principale ca sa optinem o matrice inferior triunghiulara, lucram cu operatii pe linii
                (*B.at(i,j)) = (*B.at(i,j)) - (*(A.at(i,j)))/(*(A.at(i,i))) * (*B.at(i,i));
                (*A.at(i,j)) = (*A.at(i,j)) - (*(A.at(i,j)))/(*(A.at(i,i))) * (*A.at(i,i));
            }

        B = B.transpose();

        for(int i = 0; i < this->linii; i++)

            for(int j = i; j < this->linii; j++){  
                (*B.at(i,j)) = (*B.at(i,j)) - (*(A.at(i,j)))/(*(A.at(i,i))) * (*B.at(i,i));
                (*A.at(i,j)) = (*A.at(i,j)) - (*(A.at(i,j)))/(*(A.at(i,i))) * (*A.at(i,i));
            }      
            
        for(int i = 0; i < this->linii; i++)
            for(int j = i; j < this->linii; j++){  
                (*B.at(i,j)) = (*B.at(i,j)) / (*(A->at(i,i)));

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

        if( this->coloane != B->linii ){
            errno = 1;
            Matrice O;
            return O;
        }

        int n = this->linii;
        int m = B->coloane;
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

    Matrice operator^ (char *pow) {

        if(*pow == 'T' || *pow == 't'){
            return this->transpose();
        }

        if(pow[0] == '.' ){
            return this->derivative();
        }
        
        char inverseflag;

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
    }



}
/*
matrice* produsMatrice(matrice *A, matrice *B){

    if( A->coloane != B->linii ){
        errno = 1;
        return NULL;
    }

    int n = A->linii;
    int m = B->coloane;
    int p = A->coloane;

    matrice* P;
    P = creareMatrice(n,m);

    for(int i =  0 ; i < n; i++)
        for(int j = 0; j < m; j++){
            float s = 0;
            for(int k = 0; k < p; k++)
                s += (at(A,i,k))*(at(B,k,j));
            P->valori[pos(P,i,j)] = s;
        }

    return P;
}

matrice * transpunere(matrice* A){
    matrice *T;
    T =  creareMatrice(A->coloane,A->linii);
    for(int i = 0; i < A->linii; i++)
        for( int j = 0; j < A->coloane; j++){
            T->valori(pos(T,j,i)) = at(A,i,j);
        }

    return T;
}
*/
