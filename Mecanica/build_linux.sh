#!/bin/bash

clear

echo -e "\e[36m==> Începem compilarea pe Linux...\e[0m"

g++ src/*.cpp src/glad.c -o simulator -I includes -B/usr/bin/ -L/usr/lib/x86_64-linux-gnu -lglfw -lGL -lm -ldl

if [ $? -eq 0 ]; then
    echo -e "\e[32m==> Compilare reușită! 😎 \e[0m"
    ./simulator
else
    echo -e "\e[31m==> A apărut o eroare 👿\e[0m"
fi