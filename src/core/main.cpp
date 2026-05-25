#include "sistem.h"
#include "grafica.h"
#include "fizica.h"
#include "editor.h" 
#include "newgui.h"
#include "input.h"
#include "scena.h"
#include <cmath>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const float PI = 3.1415926535f;

// 1. Definim o structură pentru a ține minte starea între cadre
struct AppState {
    sistem* S;
    editor* E;
    double timp_anterior;
    double timp_trecut;
};

// 2. Mutăm tot interiorul buclei while în această funcție
void main_loop(void* arg) {
    // Extragem starea din pachet
    AppState* app = static_cast<AppState*>(arg);
    sistem& S = *(app->S);
    editor& E = *(app->E);

    double timp;
    double dt_cadru;
    int iteratii;

    processInput(S, E);
    startFrameGUI();

    timp = glfwGetTime();
    dt_cadru = timp - app->timp_anterior;
    app->timp_anterior = timp;

    if(dt_cadru > 0.1){
        dt_cadru = 0.0f;
    }

    if(E.mod_curent == MOD_RULARE){

        app->timp_trecut += dt_cadru * S.scala_timp;
        iteratii = 0;

        for(size_t i = 0; i < S.corpuri.size(); i++){
            S.corpuri[i].forte_desen.reseteaza();
            S.corpuri[i].forte_desen.viteza_cadru_trecut = S.corpuri[i].viteza;
            S.corpuri[i].forte_desen.omega_cadru_trecut = S.corpuri[i].omega;
        }

        while(app->timp_trecut >= S.dt) {  
            S.step();
            app->timp_trecut -= S.dt;
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

int main() {
    
    sistem S; 
    editor E;
    S.setareConstantaGravitationala(9.81f);

    incarcaScenaInitiala(S);
    E.sincronizeazaMemorie(S);

    E.window = initializareGrafica(S,E);

    // 3. Inițializăm starea pe care o vom trimite către main_loop
    AppState app;
    app.S = &S;
    app.E = &E;
    app.timp_anterior = glfwGetTime();
    app.timp_trecut = 0.0f;

    // 4. Alegem cine rulează bucla: Browserul sau Desktop-ul
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop, &app, 0, true);
#else
    while(!glfwWindowShouldClose(E.window)) {
        main_loop(&app);
    }

    // Partea de salvare pfd (doar pe desktop)
    if (E.flag.salveaza_log_corpuri_la_final)
    {
        if (E.fisier_export.is_open()) {
            E.fisier_export.close();
        }

        auto f_corpuri = pfd::save_file("Salveaza log corpuri", "", {"Fisier CSV", "*.csv"}, pfd::opt::force_overwrite);
        if (!f_corpuri.result().empty()) {
            try {
                std::filesystem::copy_file(E.nume_fisier_export, f_corpuri.result(), std::filesystem::copy_options::overwrite_existing);

            } catch (const std::filesystem::filesystem_error& e) {
                pfd::message("Eroare Salvare", std::string("Nu s-a putut salva fisierul de log pentru corpuri:\n") + e.what(), pfd::choice::ok, pfd::icon::error);
            }
        }
    } 

    if (E.flag.salveaza_log_legaturi_la_final)
    {
        if (E.fisier_export_legaturi.is_open()) {
            E.fisier_export_legaturi.close();
        }
        auto f_legaturi = pfd::save_file("Salveaza log legaturi", "", {"Fisier CSV", "*.csv"}, pfd::opt::force_overwrite);
        if (!f_legaturi.result().empty()) {
            try {
                std::filesystem::copy_file(E.nume_fisier_export_legaturi, f_legaturi.result(), std::filesystem::copy_options::overwrite_existing);
            } catch (const std::filesystem::filesystem_error& e) {
                pfd::message("Eroare Salvare", std::string("Nu s-a putut salva fisierul de log pentru legaturi:\n") + e.what(), pfd::choice::ok, pfd::icon::error);
            }
        }
    }
    
    cleanupGUI();
    glDeleteVertexArrays(1, &(E.VAO));
    glDeleteBuffers(1, &(E.VBO));
    glfwTerminate();
#endif

    return 0;
}