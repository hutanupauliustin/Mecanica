#pragma once
#include "sistem.h"
#include "matrice.h"
#include "colliziune.h"

//rezolva sisteme olonoame scleronome, cu legaturi bilaterale

void calculeazaMultiplicatori(sistem &S, double t);
    
matrice derivata(sistem &S, const matrice &stare_curenta, double t);

matrice RK4(sistem &S, double dt, double t);

void adaugaForteContinueVizuale(sistem &S);

void calculeazaEnergiaTotala(sistem &S);

void salveazaDateCinematiceVizuale(sistem &S, float dt_pas_fizica, int nr_iteratii);