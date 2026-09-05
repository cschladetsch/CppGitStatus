$ErrorActionPreference = "Stop"

$destDir = "$HOME/bin"
if (!(Test-Path $destDir)) {
    New-Item -ItemType Directory -Force -Path $destDir | Out-Null
}

if (Test-Path "build/gits.exe") {
    Copy-Item "build/gits.exe" "$destDir/gits.exe" -Force
    Write-Host "Installed gits.exe to $destDir" -ForegroundColor Green
} else {
    Write-Host "Error: build/gits.exe not found." -ForegroundColor Red
    exit 1
}
