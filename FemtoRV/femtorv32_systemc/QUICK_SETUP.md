# Quick Setup Guide - No Admin Rights Required

## 🚀 **Option 1: Portable MinGW (Recommended)**

### Download and Setup:
1. **Download MinGW-w64 portable**: https://github.com/niXman/mingw-builds-binaries/releases
2. **Download the latest release** (e.g., `x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev1.7z`)
3. **Extract to**: `C:\mingw64` (or any folder you have write access to)
4. **Add to PATH**: Add `C:\mingw64\bin` to your system PATH

### Test Installation:
```cmd
g++ --version
make --version
```

## 🐧 **Option 2: Use WSL (Windows Subsystem for Linux)**

### Install WSL:
```powershell
wsl --install
```

### After WSL installation:
```bash
# Update package list
sudo apt update

# Install build tools
sudo apt install build-essential

# Install SystemC
sudo apt install libsystemc-dev

# Navigate to project
cd /mnt/c/Projects/learn-fpga/FemtoRV/femtorv32_systemc

# Build
make
```

## 🔧 **Option 3: Visual Studio Community (Alternative)**

### Download Visual Studio Community:
1. **Download**: https://visualstudio.microsoft.com/vs/community/
2. **Install with C++ workload**
3. **Use the provided .vcxproj file** (I can create this)

## 📦 **Option 4: Pre-compiled SystemC**

### Download SystemC:
1. **Download SystemC 2.3.3**: https://www.accellera.org/downloads/standards/systemc
2. **Extract to**: `C:\SystemC\systemc-2.3.3`
3. **Update build scripts** with correct paths

## 🎯 **Quick Test (No SystemC Required)**

I can create a simplified version that compiles without SystemC for testing:

```cpp
// Simple test without SystemC
#include <iostream>
#include <cstdint>

int main() {
    std::cout << "FemtoRV32 SystemC Test - Ready to build!" << std::endl;
    return 0;
}
```

## 🤔 **Which option would you prefer?**

1. **Portable MinGW** - Download and extract, no admin needed
2. **WSL** - Linux environment in Windows
3. **Visual Studio** - Microsoft's IDE
4. **Simplified test** - Just verify the code compiles
5. **Manual SystemC setup** - Download and configure manually

Let me know which option you'd like to try!
