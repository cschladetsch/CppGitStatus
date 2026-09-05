$ErrorActionPreference = "Stop"

Write-Host "==> Checking build directory..." -ForegroundColor Cyan
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Force -Path "build" | Out-Null
    Push-Location build
    cmake -G Ninja ..
    Pop-Location
}

Write-Host "==> Building project..." -ForegroundColor Cyan
cmake --build build

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "==> Running install.ps1..." -ForegroundColor Cyan
if (Test-Path ".\install.ps1") {
    .\install.ps1
} else {
    Write-Host "Error: install.ps1 not found." -ForegroundColor Red
    exit 1
}
