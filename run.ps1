param (
    [switch]$Clean
)

# 1. Curatarea fisierelor extra (LaTeX)
Write-Host "==> Curatam fisierele auxiliare..." -ForegroundColor Cyan
$docPath = "..\doc"
$extsToDelete = @("*.aux", "*.log", "*.out", "*.toc", "*.fls", "*.fdb_latexmk", "*.synctex.gz")
foreach ($ext in $extsToDelete) {
    Remove-Item -Path "$docPath\$ext" -Force -ErrorAction SilentlyContinue
}

# 1.5. Clean build complet (se ruleaza doar daca dai ./run.ps1 -Clean)
if ($Clean -and (Test-Path -Path "build")) {
    Write-Host "==> Curatam cache-ul (stergem folderul build)..." -ForegroundColor Yellow
    Remove-Item -Path "build" -Recurse -Force
}

# 2. Configurare cu CMake si Ninja
# Verificam daca Ninja a fost deja configurat in folderul build
if (!(Test-Path -Path "build\build.ninja")) {
    Write-Host "==> Initializare ..." -ForegroundColor Yellow
    
    # Aici e magia: flag-ul -G "Ninja" ii spune sa foloseasca Ninja in loc de Makefiles
    cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "==> Eroare la configurarea CMake! Verifica CMakeLists.txt." -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

# 3. Compilarea incrementala
Write-Host "==> Compilam codul ..." -ForegroundColor Cyan

cmake --build build

# 4. Rularea executabilului
if ($LASTEXITCODE -eq 0) {
    Write-Host "==> Gata! Pornim simulatorul..." -ForegroundColor Green
    .\build\simulator.exe
} else {
    Write-Host "==> Eroare la compilare! Verifica logurile de mai sus." -ForegroundColor Red
}