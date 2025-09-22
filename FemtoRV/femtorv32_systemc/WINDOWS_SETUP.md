# Windows Setup Guide for FemtoRV32 SystemC

## Prerequisites Installation

### 1. Install MinGW-w64 (GCC Compiler)

**Option A: Using MSYS2 (Recommended)**
1. Download MSYS2 from https://www.msys2.org/
2. Install and run MSYS2
3. In MSYS2 terminal, run:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-make
   ```

**Option B: Using Chocolatey**
1. Install Chocolatey: https://chocolatey.org/install
2. Run in PowerShell as Administrator:
   ```powershell
   choco install mingw
   ```

**Option C: Direct Download**
1. Download from https://www.mingw-w64.org/downloads/
2. Extract to `C:\mingw64`
3. Add `C:\mingw64\bin` to your PATH

### 2. Install SystemC

**Download and Install:**
1. Download SystemC 2.3.3 from https://www.accellera.org/downloads/standards/systemc
2. Extract to `C:\SystemC\systemc-2.3.3`
3. Build SystemC:
   ```bash
   cd C:\SystemC\systemc-2.3.3
   mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   mingw32-make
   ```

### 3. Update PATH Environment Variable

Add these to your system PATH:
- `C:\mingw64\bin` (or your MinGW installation)
- `C:\SystemC\systemc-2.3.3\lib-msvc64` (or appropriate lib directory)

## Building the Project

### Method 1: Using PowerShell Script (Recommended)
```powershell
.\build.ps1 build
.\build.ps1 run
.\build.ps1 test
```

### Method 2: Using Batch File
```cmd
build.bat build
build.bat run
build.bat test
```

### Method 3: Manual Compilation
```bash
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c main.cpp -o main.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c register_file.cpp -o register_file.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c alu.cpp -o alu.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c instruction_decoder.cpp -o instruction_decoder.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c csr_registers.cpp -o csr_registers.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c memory_interface.cpp -o memory_interface.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c femtorv32_core.cpp -o femtorv32_core.o
g++ -std=c++11 -Wall -Wextra -O2 -g -IC:\SystemC\systemc-2.3.3\include -I. -c testbench.cpp -o testbench.o
g++ main.o register_file.o alu.o instruction_decoder.o csr_registers.o memory_interface.o femtorv32_core.o testbench.o -LC:\SystemC\systemc-2.3.3\lib-msvc64 -lsystemc -lm -o femtorv32_systemc.exe
```

## Troubleshooting

### Common Issues:

1. **"g++ not found"**
   - Make sure MinGW is installed and in PATH
   - Restart your terminal after adding to PATH

2. **"SystemC not found"**
   - Update the SystemC_HOME path in build scripts
   - Make sure SystemC is built and installed

3. **"Permission denied"**
   - Run PowerShell as Administrator
   - Check file permissions

4. **"Linking errors"**
   - Verify SystemC library path
   - Check if SystemC was built correctly

## Quick Start (If you have the tools installed)

```powershell
# Build the project
.\build.ps1 build

# Run the simulation
.\build.ps1 run

# Run tests
.\build.ps1 test

# Clean build artifacts
.\build.ps1 clean
```

## Alternative: Use WSL (Windows Subsystem for Linux)

If you prefer a Linux-like environment:
1. Install WSL2
2. Install Ubuntu from Microsoft Store
3. Follow the Linux build instructions in README.md
