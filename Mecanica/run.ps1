# 1. Curatarea fisierelor extra (LaTeX)
Write-Host "==> Curatam fisierele auxiliare..." -ForegroundColor Cyan
$docPath = "..\doc"
$extsToDelete = @("*.aux", "*.log", "*.out", "*.toc", "*.fls", "*.fdb_latexmk", "*.synctex.gz")
foreach ($ext in $extsToDelete) {
    Remove-Item -Path "$docPath\$ext" -Force -ErrorAction SilentlyContinue
}

# 2. Verificam daca folderul de build exista (Configurare initiala CMake)
if (!(Test-Path -Path "build" -PathType Container)) {
    Write-Host "==> Folderul 'build' lipseste. Initializam proiectul CMake..." -ForegroundColor Yellow
    cmake -S . -B build -G "MinGW Makefiles"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "==> Eroare la configurarea CMake! Verifica fisierul CMakeLists.txt." -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

# 3. Compilarea incrementala (doar ce a fost modificat in C++)
Write-Host "==> Verificam si compilam codul C++..." -ForegroundColor Cyan
cmake --build build

# 4. Rularea executabilului (daca compilarea a reusit sau daca era deja compilat)
if ($LASTEXITCODE -eq 0) {
    Write-Host "==> Gata! Pornim simulatorul..." -ForegroundColor Green
    .\build\simulator.exe
} else {
    Write-Host "==> Eroare! Mai sus poti vedea detaliile exacte (erori de cod sau de linkare)." -ForegroundColor Red
}
