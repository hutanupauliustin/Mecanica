#!/bin/bash

clear

echo -e "\e[36m==> Începem compilarea pe Linux...\e[0m"

# Am pus -B/usr/bin/ ca să forțăm uneltele sistemului și -lglfw standard
g++ src/main.cpp src/glad.c -o simulator -I includes -B/usr/bin/ -lglfw -lGL -lm -ldl

# 3. Verificăm dacă a compilat cu succes
if [ $? -eq 0 ]; then
    echo -e "\e[32m==> Compilare reușită! 😎 \e[0m"
    ./simulator
else
    echo -e "\e[31m==> A apărut o eroare 👿\e[0m"
fi