# Makefile for FemtoRV32 SystemC Implementation
# Production-quality build system for Qualcomm interview preparation

# Compiler and flags
CXX = g++
SYSTEMC_HOME = /usr/local/systemc-2.3.3
SYSTEMC_INCLUDE = $(SYSTEMC_HOME)/include
SYSTEMC_LIB = $(SYSTEMC_HOME)/lib-linux64

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g
INCLUDES = -I$(SYSTEMC_INCLUDE) -I.
LDFLAGS = -L$(SYSTEMC_LIB) -lsystemc -lm

# Source files
SOURCES = main.cpp \
          register_file.cpp \
          alu.cpp \
          instruction_decoder.cpp \
          csr_registers.cpp \
          memory_interface.cpp \
          femtorv32_core.cpp \
          testbench.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = femtorv32_systemc

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build completed successfully!"

# Compile source files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(TARGET) *.vcd
	@echo "Clean completed!"

# Run the simulation
run: $(TARGET)
	@echo "Running FemtoRV32 SystemC simulation..."
	./$(TARGET)

# Run with Valgrind for memory checking
valgrind: $(TARGET)
	@echo "Running with Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Generate documentation
docs:
	@echo "Generating documentation..."
	doxygen Doxyfile

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing SystemC dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential cmake
	sudo apt-get install -y libsystemc-dev

# Build SystemC from source (if needed)
build-systemc:
	@echo "Building SystemC from source..."
	wget https://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tgz
	tar -xzf systemc-2.3.3.tgz
	cd systemc-2.3.3 && mkdir build && cd build
	cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/systemc-2.3.3
	make -j$(nproc)
	sudo make install

# Run tests
test: $(TARGET)
	@echo "Running test suite..."
	./$(TARGET) 2>&1 | tee test_results.log
	@echo "Test results saved to test_results.log"

# Performance profiling
profile: $(TARGET)
	@echo "Running performance profiling..."
	gprof $(TARGET) gmon.out > profile.txt
	@echo "Profile results saved to profile.txt"

# Static analysis
analyze:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c++11 *.cpp *.h
	@echo "Static analysis completed!"

# Format code
format:
	@echo "Formatting code..."
	clang-format -i *.cpp *.h
	@echo "Code formatting completed!"

# Create release package
package: clean
	@echo "Creating release package..."
	tar -czf femtorv32_systemc.tar.gz *.cpp *.h *.md Makefile
	@echo "Release package created: femtorv32_systemc.tar.gz"

# Help target
help:
	@echo "FemtoRV32 SystemC Makefile"
	@echo "=========================="
	@echo "Available targets:"
	@echo "  all          - Build the executable (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  run          - Build and run the simulation"
	@echo "  valgrind     - Run with Valgrind memory checking"
	@echo "  test         - Run the test suite"
	@echo "  profile      - Run performance profiling"
	@echo "  analyze      - Run static code analysis"
	@echo "  format       - Format source code"
	@echo "  docs         - Generate documentation"
	@echo "  package      - Create release package"
	@echo "  install-deps - Install SystemC dependencies"
	@echo "  build-systemc- Build SystemC from source"
	@echo "  help         - Show this help message"

# Phony targets
.PHONY: all clean run valgrind test profile analyze format docs package install-deps build-systemc help

# Dependencies
main.o: main.cpp testbench.h femtorv32_core.h
register_file.o: register_file.cpp register_file.h femtorv32_systemc.h
alu.o: alu.cpp alu.h femtorv32_systemc.h
instruction_decoder.o: instruction_decoder.cpp instruction_decoder.h femtorv32_systemc.h
csr_registers.o: csr_registers.cpp csr_registers.h femtorv32_systemc.h
memory_interface.o: memory_interface.cpp memory_interface.h femtorv32_systemc.h
femtorv32_core.o: femtorv32_core.cpp femtorv32_core.h register_file.h alu.h instruction_decoder.h csr_registers.h memory_interface.h femtorv32_systemc.h
testbench.o: testbench.cpp testbench.h femtorv32_core.h