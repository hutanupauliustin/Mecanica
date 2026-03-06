param (
    # Daca nu scrii niciun nume in terminal, va folosi "simulator.exe" by default
    [string]$NumeExe = "simulator.exe"
)

Clear-Host #facut de AI

Write-Host "==> Începem compilarea pentru $NumeExe..." -ForegroundColor Cyan

# Gaseste automat toate fisierele .cpp si .c din directorul src
$sourceFiles = Get-ChildItem -Path src -Recurse -Include *.cpp, *.c | ForEach-Object { $_.FullName }


Write-Host "Compilam fisierele: $sourceFiles"

# Am pus $NumeExe in loc de simulator.exe
g++ $sourceFiles -o $NumeExe -O3 -I includes -L lib -lglfw3 -lgdi32 -lopengl32 -static -static-libgcc -static-libstdc++


if ($?) {
    Write-Host "==> Compilare reusita!" -ForegroundColor Green
    
    # Am pus operatorul & (call) ca sa stie PowerShell sa ruleze variabila ca pe un program
    & ".\$NumeExe"
} else {
    Write-Host "==> A aparut o eroare!" -ForegroundColor Red
}