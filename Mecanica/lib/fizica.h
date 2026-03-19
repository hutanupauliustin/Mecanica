#pragma once
#include "sistem.h"
#include "matrice.h"
#include "colliziune.h"

//rezolva sisteme olonoame scleronome, cu legaturi bilaterale

void calculeazaMultiplicatori(sistem &S, float t);
    
matrice derivata(sistem &S, const matrice &stare_curenta, float t);

matrice RK4(sistem &S, float dt, float t);

float calculeazaEnergiaTotala(sistem &S, float g);