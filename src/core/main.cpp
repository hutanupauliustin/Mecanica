#include "sistem.h"
#include "grafica.h"
#include "fizica.h"
#include "editor.h" 
#include "gui.h"
#include "scena.h"
#include "editor.h"
#include <cmath>
#include <iostream>

const float PI = 3.1415926535f;

int main() {
    
    sistem S; 
    editor E;
    S.setareConstantaGravitationala(9.81f);

    incarcaScenaInitiala(S);
    E.sincronizeazaMemorie(S);

    GLFWwindow* window = initializareGrafica(S,E);

    double timp_anterior = glfwGetTime();
    //double timp_offset = 0.0f;
    double timp_trecut = 0.0f;
    double timp;
    double dt_cadru;
    int iteratii;
        
    while(!glfwWindowShouldClose(window)) {

        processInput(window, S, E);
        startFrameGUI();

        timp = glfwGetTime();
        dt_cadru = timp - timp_anterior;
        timp_anterior = timp;

        if(dt_cadru > 0.1){
            dt_cadru = 0.0f;
        }

        if(E.mod_curent == MOD_RULARE){

            timp_trecut += dt_cadru * S.scala_timp;
            iteratii = 0;

            for(size_t i = 0; i < S.corpuri.size(); i++){
                S.corpuri[i].forte_desen.reseteaza();
                S.corpuri[i].forte_desen.viteza_cadru_trecut = S.corpuri[i].viteza;
                S.corpuri[i].forte_desen.omega_cadru_trecut = S.corpuri[i].omega;
            }

            while(timp_trecut >= S.dt) {  
                S.step(E.dt);
               
                S.t += S.dt;
                timp_trecut -= S.dt;
                iteratii++;
            }


            adaugaForteContinueVizuale(S);
            salveazaDateCinematiceVizuale(S,E.dt,iteratii);
            
            E.incarcaDatePentruGrafic(S);

            } else {
                S.incarcaStare();
                verificarCiocniri(S,E);
            }
           
            if(E.flag.arata_energie){
                calculeazaEnergiaTotala(S);
        }
        
        randareGrafica(S,E,window);
           
        }
        
        cleanupGUI();
        glDeleteVertexArrays(1, &(E.VAO));
        glDeleteBuffers(1, &(E.VBO));
        glfwTerminate();

        return 0;
    }
