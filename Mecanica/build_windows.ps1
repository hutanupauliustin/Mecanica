param (
    # Daca nu scrii niciun nume in terminal, va folosi "simulator.exe" by default
    [string]$NumeExe = "simulator.exe"
)

Clear-Host

Write-Host "==> Începem compilarea pentru $NumeExe..." -ForegroundColor Cyan

$sourceFiles = Get-ChildItem -Path "src", "imgui" -Recurse -Include *.cpp, *.c | ForEach-Object { $_.FullName }


Write-Host "Compilam fisierele: $sourceFiles"

g++ $sourceFiles -o $NumeExe -O3 -I includes -I imgui -L lib -lglfw3 -lgdi32 -lopengl32 -static -static-libgcc -static-libstdc++


if ($?) {
    Write-Host "==> Compilare reusita!" -ForegroundColor Green
    
    & ".\$NumeExe"
} else {
    Write-Host "==> A aparut o eroare!" -ForegroundColor Red
}