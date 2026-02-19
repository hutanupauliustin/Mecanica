Clear-Host

Write-Host "==> Începem compilarea..." -ForegroundColor Cyan

g++ src/main.cpp src/glad.c -o simulator.exe -I includes -L lib -lglfw3 -lgdi32 -lopengl32

if ($?) {
    Write-Host "==> Compilare reusita!" -ForegroundColor Green
    .\simulator.exe
} else {
    Write-Host "==> A aparut o eroare!" -ForegroundColor Red
}