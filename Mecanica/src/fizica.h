#pragma once
#include "sistem.h"

matrice f(const matrice &x, float t);
void seteazaForte(sistem &S, float t);
matrice derivata(sistem &S, const matrice &stare_curenta, float t);
matrice RK4(sistem &S, const matrice &x, float dt, float t);