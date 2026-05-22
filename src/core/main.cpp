#include "sistem.h"
#include "grafica.h"
#include "fizica.h"
#include "editor.h" 
#include "newgui.h"
#include "input.h"
#include "scena.h"
#include <cmath>
#include <iostream>

const float PI = 3.1415926535f;

int main() {
    
    sistem S; 
    editor E;
    S.setareConstantaGravitationala(9.81f);

    incarcaScenaInitiala(S);
    E.sincronizeazaMemorie(S);

    E.window = initializareGrafica(S,E);

    double timp_anterior = glfwGetTime();
    //double timp_offset = 0.0f;
    double timp_trecut = 0.0f;
    double timp;
    double dt_cadru;
    int iteratii;
        
    while(!glfwWindowShouldClose(E.window)) {

        processInput(S, E);
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
                S.step();
                timp_trecut -= S.dt;
                iteratii++;
            }


            adaugaForteContinueVizuale(S);
            salveazaDateCinematiceVizuale(S,S.dt,iteratii);
            
            E.incarcaDatePentruGrafic(S);

            } else {
                S.incarcaStare();
                verificarCiocniri(S);
            }
           
            if(E.flag.arata_energie){
                calculeazaEnergiaTotala(S);
        }
        
        randareGrafica(S,E);
           
        }

        if (E.flag.salveaza_log_corpuri_la_final)
        {
            // Inchidem fisierele pentru a ne asigura ca totul este scris pe disk
            if (E.fisier_export.is_open()) {
                E.fisier_export.close();
            }

            // Deschidem dialogul "Save As" pentru corpuri
            auto f_corpuri = pfd::save_file("Salveaza log corpuri", "", {"Fisier CSV", "*.csv"}, pfd::opt::force_overwrite);
            if (!f_corpuri.result().empty()) {
                try {
                    std::filesystem::copy_file(E.nume_fisier_export, f_corpuri.result(), std::filesystem::copy_options::overwrite_existing);

                } catch (const std::filesystem::filesystem_error& e) {
                    pfd::message("Eroare Salvare", std::string("Nu s-a putut salva fisierul de log pentru corpuri:\n") + e.what(), pfd::choice::ok, pfd::icon::error);
                }
            }

        } else {
            ;   //sterge fisierul temporar
        }

        if (E.flag.salveaza_log_legaturi_la_final)
        {
            // Inchidem fisierele pentru a ne asigura ca totul este scris pe disk

            if (E.fisier_export_legaturi.is_open()) {
                E.fisier_export_legaturi.close();
            }
            // Deschidem dialogul "Save As" pentru legaturi
            auto f_legaturi = pfd::save_file("Salveaza log legaturi", "", {"Fisier CSV", "*.csv"}, pfd::opt::force_overwrite);
            if (!f_legaturi.result().empty()) {
                try {
                    std::filesystem::copy_file(E.nume_fisier_export_legaturi, f_legaturi.result(), std::filesystem::copy_options::overwrite_existing);
                } catch (const std::filesystem::filesystem_error& e) {
                    pfd::message("Eroare Salvare", std::string("Nu s-a putut salva fisierul de log pentru legaturi:\n") + e.what(), pfd::choice::ok, pfd::icon::error);
                }
            }
        } else {
            ; //sterge fisierul temporar
        }
        
        cleanupGUI();
        glDeleteVertexArrays(1, &(E.VAO));
        glDeleteBuffers(1, &(E.VBO));
        glfwTerminate();

        return 0;
    }
