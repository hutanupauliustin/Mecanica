#pragma once
#include "sistem.h"
#include "matrice.h"
#include <cmath>

bool intersectareScaraLarga(sistem &S, int corpA, int corpB);
struct Vec2
{
    float x, y;
};

float produs_vect(Vec2 v1, Vec2 v2);

Vec2 scade(Vec2 a, Vec2 b);

Vec2 aduna(Vec2 a, Vec2 b);

Vec2 inmulteste(Vec2 a, float s);

struct intersectie
{
    float adancimee;
    Vec2 normala;
    bool seLovesc;
};

struct Latura
{
    Vec2 p1, p2; // Cele doua capete ale laturii
    Vec2 n;      // Normala laturii
};

intersectie intersectareScaraMica(sistem &S, int corpA, int corpB);

Latura gasesteFataSuport(sistem &S, int corp, Vec2 directie);

// Functia generala de taiere cu un plan (rezolva ecuatiile factorului de interpolare 't')
int taierePlan(Vec2 intrari[2], int nrIntrari, Vec2 iesiri[2], Vec2 punctPlan, Vec2 normalaPlan);

// Structura care va salva 1 sau 2 puncte si adancimile lor pentru a fi folosite la aplicarea impulsului
struct PuncteContact
{
    Vec2 puncte[2];
    float adancimi[2];
    int nrPuncte;
};

PuncteContact extrageManifold(Latura ref, Latura inc);

void ciocnire(sistem &S, int corpA, int corpB, intersectie inter);

void verificarCiocniri(sistem &S);
