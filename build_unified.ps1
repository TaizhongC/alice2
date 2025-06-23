param(
    [string]$Target = "native",  # "native", "web", or "both"
    [string]$Command = "build",  # "build", "clean", "rebuild"
    [string]$Config = "Release", # "Debug" or "Release"
    [switch]$NoWarning
)

$ErrorActionPreference = "Stop"

# Build directories
$NativeDir = "_build_native"
$WebDir = "_build_web"
$OutputDir = "_output"
$NativeOutput = "$OutputDir/native"
$WebOutput = "$OutputDir/web"

function Write-BuildHeader {
    param([string]$Title)
    Write-Host ""
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host " $Title" -ForegroundColor Cyan
    Write-Host "=" * 60 -ForegroundColor Cyan
}

function Ensure-Directory {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Remove-Directory {
    param([string]$Path)
    if (Test-Path $Path) {
        Remove-Item $Path -Recurse -Force
        Write-Host "Cleaned: $Path" -ForegroundColor Yellow
    }
}

function Build-Native {
    Write-BuildHeader "Building Native Alice 2"
    
    Ensure-Directory $NativeDir
    Ensure-Directory $NativeOutput
    
    # Configure
    Write-Host "Configuring native build..." -ForegroundColor Green
    $cmakeArgs = @(
        "-S", ".",
        "-B", $NativeDir,
        "-DCMAKE_BUILD_TYPE=$Config"
    )
    
    if ($NoWarning) {
        $cmakeArgs += "-Wno-dev"
    }
    
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "Native CMake configuration failed" }
    
    # Build
    Write-Host "Building native executable..." -ForegroundColor Green
    cmake --build $NativeDir --config $Config
    if ($LASTEXITCODE -ne 0) { throw "Native build failed" }
    
    # Copy outputs
    Write-Host "Copying native outputs..." -ForegroundColor Green
    $nativeExe = Get-ChildItem -Path "$NativeDir" -Name "alice2_unified*" -Recurse | Select-Object -First 1
    if ($nativeExe) {
        Copy-Item "$NativeDir/$nativeExe" -Destination $NativeOutput -Force
        Write-Host "Native build completed: $NativeOutput/$nativeExe" -ForegroundColor Green
    }
}

function Build-Web {
    Write-BuildHeader "Building Web Alice 2"
    
    # Check for Emscripten
    try {
        $emscriptenVersion = & emcc --version 2>$null
        Write-Host "Using Emscripten: $($emscriptenVersion[0])" -ForegroundColor Blue
    }
    catch {
        throw "Emscripten not found. Please install and activate the Emscripten SDK."
    }
    
    Ensure-Directory $WebDir
    Ensure-Directory $WebOutput
    
    # Configure
    Write-Host "Configuring web build..." -ForegroundColor Green
    $cmakeArgs = @(
        "-S", ".",
        "-B", $WebDir,
        "-DCMAKE_BUILD_TYPE=$Config"
    )
    
    if ($NoWarning) {
        $cmakeArgs += "-Wno-dev"
    }
    
    & emcmake cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "Web CMake configuration failed" }
    
    # Build
    Write-Host "Building web application..." -ForegroundColor Green
    cmake --build $WebDir --config $Config
    if ($LASTEXITCODE -ne 0) { throw "Web build failed" }
    
    # Copy web outputs
    Write-Host "Copying web outputs..." -ForegroundColor Green
    $webFiles = @("alice2_web.html", "alice2_web.js", "alice2_web.wasm")
    $assetFiles = @("ui/controls.js", "ui/styles.css")
    
    foreach ($file in $webFiles) {
        $sourcePath = "$WebDir/bin/$file"
        if (Test-Path $sourcePath) {
            Copy-Item $sourcePath -Destination $WebOutput -Force
        }
    }
    
    foreach ($asset in $assetFiles) {
        $sourcePath = "$WebDir/bin/$asset"
        $targetDir = Join-Path $WebOutput (Split-Path -Parent $asset)
        if (Test-Path $sourcePath) {
            Ensure-Directory $targetDir
            Copy-Item $sourcePath -Destination (Join-Path $WebOutput $asset) -Force
        }
    }
    
    Write-Host "Web build completed: $WebOutput/alice2_web.html" -ForegroundColor Green
}

function Clean-All {
    Write-BuildHeader "Cleaning Build Directories"
    
    Remove-Directory $NativeDir
    Remove-Directory $WebDir
    Remove-Directory $OutputDir
    
    Write-Host "Clean completed" -ForegroundColor Green
}

function Show-Usage {
    Write-Host "Alice 2 Unified Build System" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\build_unified.ps1 [options]" -ForegroundColor White
    Write-Host ""
    Write-Host "Options:" -ForegroundColor Yellow
    Write-Host "  -Target <target>   Build target: 'native', 'web', or 'both' (default: native)"
    Write-Host "  -Command <cmd>     Command: 'build', 'clean', or 'rebuild' (default: build)"
    Write-Host "  -Config <config>   Build configuration: 'Debug' or 'Release' (default: Release)"
    Write-Host "  -NoWarning         Suppress CMake warnings"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Green
    Write-Host "  .\build_unified.ps1                    # Build native version"
    Write-Host "  .\build_unified.ps1 -Target web        # Build web version"
    Write-Host "  .\build_unified.ps1 -Target both       # Build both versions"
    Write-Host "  .\build_unified.ps1 -Command clean     # Clean all builds"
    Write-Host "  .\build_unified.ps1 -Command rebuild   # Clean and rebuild"
}

# Main execution
try {
    switch ($Command.ToLower()) {
        "clean" { 
            Clean-All
        }
        "rebuild" { 
            Clean-All
            if ($Target -eq "both") {
                Build-Native
                Build-Web
            } elseif ($Target -eq "web") {
                Build-Web
            } else {
                Build-Native
            }
        }
        "build" { 
            if ($Target -eq "both") {
                Build-Native
                Build-Web
            } elseif ($Target -eq "web") {
                Build-Web
            } elseif ($Target -eq "native") {
                Build-Native
            } else {
                Show-Usage
                throw "Invalid target: $Target. Use 'native', 'web', or 'both'."
            }
        }
        "help" {
            Show-Usage
            exit 0
        }
        default { 
            Show-Usage
            throw "Unknown command: $Command. Use 'build', 'clean', 'rebuild', or 'help'."
        }
    }
    
    Write-Host ""
    Write-Host "Build operation completed successfully!" -ForegroundColor Green
    
    if ($Target -eq "web" -or $Target -eq "both") {
        Write-Host ""
        Write-Host "To run the web version:" -ForegroundColor Cyan
        Write-Host "  1. Start a local web server in the $WebOutput directory"
        Write-Host "  2. Open alice2_web.html in your browser"
        Write-Host "  Example: python -m http.server 8000 (then visit http://localhost:8000)"
    }
}
catch {
    Write-Host ""
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
}
