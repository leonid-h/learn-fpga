/**
 * @file alu.h
 * @brief RISC-V ALU SystemC Module
 * 
 * Implements the Arithmetic Logic Unit with support for:
 * - Basic arithmetic operations (ADD, SUB)
 * - Logical operations (AND, OR, XOR)
 * - Shift operations (SLL, SRL, SRA)
 * - Comparison operations (SLT, SLTU)
 * - Multiplication and division (RV32M)
 * - Proper SystemC timing and pipelining
 */

#ifndef ALU_H
#define ALU_H

#include "femtorv32_systemc.h"

/**
 * @brief ALU Module
 * 
 * Implements the Arithmetic Logic Unit with support for all RISC-V operations
 * including multiplication and division with proper timing modeling.
 */
class ALU : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Input operands
    sc_in<sc_uint<32>> operand1;
    sc_in<sc_uint<32>> operand2;
    sc_in<ALUOperation> operation;
    sc_in<bool> start;
    
    // Output
    sc_out<sc_uint<32>> result;
    sc_out<bool> busy;
    sc_out<bool> valid;
    
    // Constructor
    SC_CTOR(ALU) {
        // Register processes
        SC_METHOD(combinational_operations);
        sensitive << operand1 << operand2 << operation << start;
        
        SC_METHOD(sequential_operations);
        sensitive << clk.pos();
        
        // Initialize internal state
        internal_result = 0;
        division_busy = false;
        division_cycles = 0;
        valid.write(false);
        busy.write(false);
    }
    
private:
    // Internal state
    sc_uint<32> internal_result;
    bool division_busy;
    int division_cycles;
    
    // Division state
    sc_uint<32> dividend;
    sc_uint<32> divisor;
    sc_uint<32> quotient;
    sc_uint<32> remainder;
    bool div_sign;
    
    // Process methods
    void combinational_operations();
    void sequential_operations();
    
    // Helper methods
    sc_uint<32> barrel_shift_left(sc_uint<32> value, sc_uint<5> shift_amount);
    sc_uint<32> barrel_shift_right(sc_uint<32> value, sc_uint<5> shift_amount, bool arithmetic);
    sc_uint<32> multiply_32x32(sc_uint<32> a, sc_uint<32> b, bool signed_a, bool signed_b);
    void start_division(sc_uint<32> dividend_val, sc_uint<32> divisor_val, bool signed_div);
    void step_division();
    sc_uint<32> get_division_result(bool is_remainder);
};

#endif // ALU_H
