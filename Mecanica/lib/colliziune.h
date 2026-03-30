#pragma once
#include "sistem.h"
#include "matrice.h"
#include "editor.h"
#include <cmath>

bool intersectareScaraLarga(sistem &S, int corpA, int corpB);

struct intersectie
{
    float adancimee;
    vec2 normala;
    bool seLovesc;
};

struct Latura
{
    vec2 p1, p2; // Cele doua capete ale laturii
    vec2 n;      // Normala laturii
};

intersectie intersectareScaraMica(sistem &S, int corpA, int corpB);

Latura gasesteFataSuport(sistem &S, int corp, vec2 directie);

// Functia generala de taiere cu un plan (rezolva ecuatiile factorului de interpolare 't')
int taierePlan(vec2 intrari[2], int nrIntrari, vec2 iesiri[2], vec2 punctPlan, vec2 normalaPlan);

// Structura care va salva 1 sau 2 puncte si adancimile lor pentru a fi folosite la aplicarea impulsului
struct PuncteContact
{
    vec2 puncte[2];
    float adancimi[2];
    int nrPuncte;
};

PuncteContact extrageManifold(Latura ref, Latura inc);

void ciocnire(sistem &S, int corpA, int corpB, intersectie inter);

void verificarCiocniri(sistem &S, editor &E);

void adaugaFortePercutanteVizuale(sistem &S);
