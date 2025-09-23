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
FOCUSED_TEST_SOURCES = tests/focused_test.cpp femtorv32_quark.cpp
FOCUSED_TEST_OBJECTS = $(FOCUSED_TEST_SOURCES:.cpp=.o)
FOCUSED_TEST_TARGET = tests/focused_test

SIMPLE_BRANCH_TEST_SOURCES = tests/simple_branch_test.cpp femtorv32_quark.cpp
SIMPLE_BRANCH_TEST_OBJECTS = $(SIMPLE_BRANCH_TEST_SOURCES:.cpp=.o)
SIMPLE_BRANCH_TEST_TARGET = tests/simple_branch_test

# Default target
all: $(FOCUSED_TEST_TARGET)

# Build the focused test executable (comprehensive testing)
$(FOCUSED_TEST_TARGET): $(FOCUSED_TEST_OBJECTS)
	$(CXX) $(FOCUSED_TEST_OBJECTS) -o $(FOCUSED_TEST_TARGET) $(LDFLAGS)

# Build the simple branch test executable
$(SIMPLE_BRANCH_TEST_TARGET): $(SIMPLE_BRANCH_TEST_OBJECTS)
	$(CXX) $(SIMPLE_BRANCH_TEST_OBJECTS) -o $(SIMPLE_BRANCH_TEST_TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(FOCUSED_TEST_OBJECTS) $(FOCUSED_TEST_TARGET) $(SIMPLE_BRANCH_TEST_OBJECTS) $(SIMPLE_BRANCH_TEST_TARGET) *.vcd

# Run the comprehensive assembly instruction tests
test: $(FOCUSED_TEST_TARGET)
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH ./tests/focused_test

# Run the simple branch test
simple-branch-test: $(SIMPLE_BRANCH_TEST_TARGET)
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH ./tests/simple_branch_test

# Debug build
debug: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -g -DDEBUG" $(FOCUSED_TEST_TARGET)

# Debug run (build with debug symbols and launch gdb)
debug-run: debug
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH cgdb ./tests/focused_test

# Run with Valgrind memory checker
valgrind: $(FOCUSED_TEST_TARGET)
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./tests/focused_test

# Run branch test with Valgrind
valgrind-branch: $(SIMPLE_BRANCH_TEST_TARGET)
	LD_LIBRARY_PATH=./systemc-install/lib:$$LD_LIBRARY_PATH valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./tests/simple_branch_test

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build the test suite (default)"
	@echo "  clean         - Remove build artifacts"
	@echo "  test          - Build and run comprehensive assembly instruction tests"
	@echo "  simple-branch-test - Build and run simple branch verification test"
	@echo "  debug         - Build with debug symbols"
	@echo "  debug-run     - Build with debug symbols and launch gdb"
	@echo "  valgrind      - Run focused test with Valgrind memory checker"
	@echo "  valgrind-branch - Run branch test with Valgrind memory checker"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Note: All warnings are treated as errors (-Werror flag enabled)"

.PHONY: all clean test simple-branch-test debug debug-run valgrind valgrind-branch help
