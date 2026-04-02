#pragma once
#include "sistem.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

void incarcaScenaInitiala(sistem &S);

void salveazaScenaJSON(sistem &S, const std::string& nume_fisier);

void citesteScenaJSON(sistem &S, const std::string& nume_fisier);