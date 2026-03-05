Clear-Host #facut de AI

Write-Host "==> Începem compilarea..." -ForegroundColor Cyan

# Gaseste automat toate fisierele .cpp si .c din directorul src
$sourceFiles = Get-ChildItem -Path src -Recurse -Include *.cpp, *.c | ForEach-Object { $_.FullName }

Write-Host "Compilam fisierele: $sourceFiles"

g++ $sourceFiles -o simulator.exe -I includes -L lib -lglfw3 -lgdi32 -lopengl32

if ($?) {
    Write-Host "==> Compilare reusita!" -ForegroundColor Green
    .\simulator.exe
} else {
    Write-Host "==> A aparut o eroare!" -ForegroundColor Red
}

