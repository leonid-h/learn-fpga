@echo off
REM Batch Build Script for FemtoRV32 SystemC Implementation
REM Windows-compatible build system

set SystemC_HOME=C:\SystemC\systemc-2.3.3
set SystemC_INCLUDE=%SystemC_HOME%\include
set SystemC_LIB=%SystemC_HOME%\lib-msvc64

set CXX=g++
set CXXFLAGS=-std=c++11 -Wall -Wextra -O2 -g
set INCLUDES=-I%SystemC_INCLUDE% -I.
set LDFLAGS=-L%SystemC_LIB% -lsystemc -lm

set TARGET=femtorv32_systemc.exe

if "%1"=="clean" goto clean
if "%1"=="run" goto run
if "%1"=="test" goto test
if "%1"=="help" goto help

:build
echo Building FemtoRV32 SystemC Implementation...

REM Check if SystemC is installed
if not exist "%SystemC_INCLUDE%" (
    echo Error: SystemC not found at %SystemC_INCLUDE%
    echo Please install SystemC or update the SystemC_HOME path in this script
    pause
    exit /b 1
)

REM Compile source files
echo Compiling source files...
%CXX% %CXXFLAGS% %INCLUDES% -c main.cpp -o main.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c register_file.cpp -o register_file.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c alu.cpp -o alu.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c instruction_decoder.cpp -o instruction_decoder.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c csr_registers.cpp -o csr_registers.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c memory_interface.cpp -o memory_interface.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c femtorv32_core.cpp -o femtorv32_core.o
if errorlevel 1 goto error

%CXX% %CXXFLAGS% %INCLUDES% -c testbench.cpp -o testbench.o
if errorlevel 1 goto error

REM Link executable
echo Linking %TARGET%...
%CXX% main.o register_file.o alu.o instruction_decoder.o csr_registers.o memory_interface.o femtorv32_core.o testbench.o %LDFLAGS% -o %TARGET%
if errorlevel 1 goto error

echo Build completed successfully!
goto end

:clean
echo Cleaning build artifacts...
del *.o 2>nul
del %TARGET% 2>nul
del *.vcd 2>nul
echo Clean completed!
goto end

:run
if not exist "%TARGET%" (
    echo Executable not found. Building first...
    call %0 build
    if errorlevel 1 goto error
)
echo Running FemtoRV32 SystemC simulation...
%TARGET%
goto end

:test
if not exist "%TARGET%" (
    echo Executable not found. Building first...
    call %0 build
    if errorlevel 1 goto error
)
echo Running test suite...
%TARGET% > test_results.log 2>&1
echo Test results saved to test_results.log
goto end

:help
echo FemtoRV32 SystemC Build Script
echo ==============================
echo.
echo Usage: build.bat [Action]
echo.
echo Actions:
echo   build    - Build the project (default)
echo   clean    - Clean build artifacts
echo   run      - Build and run the simulation
echo   test     - Build and run tests
echo   help     - Show this help message
echo.
echo Examples:
echo   build.bat build
echo   build.bat run
echo   build.bat clean
goto end

:error
echo Build failed!
pause
exit /b 1

:end
