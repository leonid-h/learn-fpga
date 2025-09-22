# Makefile for FemtoRV32 Quark SystemC Implementation

# SystemC installation path
SYSTEMC_HOME = ./systemc-install
SYSTEMC_INCLUDE = $(SYSTEMC_HOME)/include
SYSTEMC_LIB = $(SYSTEMC_HOME)/lib

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -O2
INCLUDES = -I$(SYSTEMC_INCLUDE)
LDFLAGS = -L$(SYSTEMC_LIB) -lsystemc -lm

# Source files
SIMPLE_FIXED_TEST_SOURCES = tests/simple_fixed_test.cpp femtorv32_quark.cpp
SIMPLE_FIXED_TEST_OBJECTS = $(SIMPLE_FIXED_TEST_SOURCES:.cpp=.o)
SIMPLE_FIXED_TEST_TARGET = tests/simple_fixed_test

# Default target
all: $(SIMPLE_FIXED_TEST_TARGET)

# Build the simple fixed test executable (comprehensive testing)
$(SIMPLE_FIXED_TEST_TARGET): $(SIMPLE_FIXED_TEST_OBJECTS)
	$(CXX) $(SIMPLE_FIXED_TEST_OBJECTS) -o $(SIMPLE_FIXED_TEST_TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(SIMPLE_FIXED_TEST_OBJECTS) $(SIMPLE_FIXED_TEST_TARGET) *.vcd

# Run the comprehensive assembly instruction tests
test: $(SIMPLE_FIXED_TEST_TARGET)
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH ./tests/simple_fixed_test

# Debug build
debug: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -g -DDEBUG" $(SIMPLE_FIXED_TEST_TARGET)

# Debug run (build with debug symbols and launch gdb)
debug-run: debug
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH cgdb ./tests/simple_fixed_test

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build the test suite (default)"
	@echo "  clean         - Remove build artifacts"
	@echo "  test          - Build and run comprehensive assembly instruction tests"
	@echo "  debug         - Build with debug symbols"
	@echo "  debug-run     - Build with debug symbols and launch gdb"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Note: All warnings are treated as errors (-Werror flag enabled)"

.PHONY: all clean test debug debug-run help
