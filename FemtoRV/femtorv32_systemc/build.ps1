# PowerShell Build Script for FemtoRV32 SystemC Implementation
# Windows-compatible build system

param(
    [string]$Action = "build",
    [string]$Config = "Release"
)

# Configuration
$SystemC_HOME = "C:\SystemC\systemc-2.3.3"
$SystemC_INCLUDE = "$SystemC_HOME\include"
$SystemC_LIB = "$SystemC_HOME\lib-msvc64"

# Compiler settings
$CXX = "g++"
$CXXFLAGS = @("-std=c++11", "-Wall", "-Wextra", "-O2", "-g")
$INCLUDES = @("-I$SystemC_INCLUDE", "-I.")
$LDFLAGS = @("-L$SystemC_LIB", "-lsystemc", "-lm")

# Source files
$SOURCES = @(
    "main.cpp",
    "register_file.cpp",
    "alu.cpp", 
    "instruction_decoder.cpp",
    "csr_registers.cpp",
    "memory_interface.cpp",
    "femtorv32_core.cpp",
    "testbench.cpp"
)

# Object files
$OBJECTS = $SOURCES | ForEach-Object { $_.Replace(".cpp", ".o") }

# Target executable
$TARGET = "femtorv32_systemc.exe"

function Build-Project {
    Write-Host "Building FemtoRV32 SystemC Implementation..." -ForegroundColor Green
    
    # Check if SystemC is installed
    if (-not (Test-Path $SystemC_INCLUDE)) {
        Write-Host "Error: SystemC not found at $SystemC_INCLUDE" -ForegroundColor Red
        Write-Host "Please install SystemC or update the SystemC_HOME path in this script" -ForegroundColor Yellow
        return $false
    }
    
    # Compile source files
    foreach ($source in $SOURCES) {
        $object = $source.Replace(".cpp", ".o")
        Write-Host "Compiling $source..." -ForegroundColor Cyan
        
        $compileCmd = @($CXX) + $CXXFLAGS + $INCLUDES + @("-c", $source, "-o", $object)
        & $compileCmd[0] $compileCmd[1..($compileCmd.Length-1)]
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Error compiling $source" -ForegroundColor Red
            return $false
        }
    }
    
    # Link executable
    Write-Host "Linking $TARGET..." -ForegroundColor Cyan
    $linkCmd = @($CXX) + $OBJECTS + $LDFLAGS + @("-o", $TARGET)
    & $linkCmd[0] $linkCmd[1..($linkCmd.Length-1)]
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error linking $TARGET" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Build completed successfully!" -ForegroundColor Green
    return $true
}

function Clean-Project {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    
    # Remove object files
    foreach ($obj in $OBJECTS) {
        if (Test-Path $obj) {
            Remove-Item $obj
            Write-Host "Removed $obj" -ForegroundColor Gray
        }
    }
    
    # Remove executable
    if (Test-Path $TARGET) {
        Remove-Item $TARGET
        Write-Host "Removed $TARGET" -ForegroundColor Gray
    }
    
    # Remove VCD files
    Get-ChildItem -Name "*.vcd" | ForEach-Object { Remove-Item $_ }
    
    Write-Host "Clean completed!" -ForegroundColor Green
}

function Run-Project {
    if (-not (Test-Path $TARGET)) {
        Write-Host "Executable not found. Building first..." -ForegroundColor Yellow
        if (-not (Build-Project)) {
            return
        }
    }
    
    Write-Host "Running FemtoRV32 SystemC simulation..." -ForegroundColor Green
    & ".\$TARGET"
}

function Test-Project {
    if (-not (Test-Path $TARGET)) {
        Write-Host "Executable not found. Building first..." -ForegroundColor Yellow
        if (-not (Build-Project)) {
            return
        }
    }
    
    Write-Host "Running test suite..." -ForegroundColor Green
    & ".\$TARGET" 2>&1 | Tee-Object -FilePath "test_results.log"
    Write-Host "Test results saved to test_results.log" -ForegroundColor Cyan
}

function Show-Help {
    Write-Host "FemtoRV32 SystemC Build Script" -ForegroundColor Green
    Write-Host "==============================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage: .\build.ps1 [Action]"
    Write-Host ""
    Write-Host "Actions:"
    Write-Host "  build    - Build the project (default)"
    Write-Host "  clean    - Clean build artifacts"
    Write-Host "  run      - Build and run the simulation"
    Write-Host "  test     - Build and run tests"
    Write-Host "  help     - Show this help message"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build.ps1 build"
    Write-Host "  .\build.ps1 run"
    Write-Host "  .\build.ps1 clean"
}

# Main execution
switch ($Action.ToLower()) {
    "build" { Build-Project }
    "clean" { Clean-Project }
    "run" { Run-Project }
    "test" { Test-Project }
    "help" { Show-Help }
    default { 
        Write-Host "Unknown action: $Action" -ForegroundColor Red
        Show-Help
    }
}
