/**
 * @file alu.cpp
 * @brief RISC-V ALU SystemC Module Implementation
 */

#include "alu.h"

void ALU::combinational_operations() {
    if (!start.read()) {
        valid.write(false);
        return;
    }
    
    sc_uint<32> op1 = operand1.read();
    sc_uint<32> op2 = operand2.read();
    ALUOperation op = operation.read();
    sc_uint<32> result_val = 0;
    
    // Handle combinational operations
    switch (op) {
        case ALUOperation::ADD:
            result_val = op1 + op2;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::SUB:
            result_val = op1 - op2;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::SLL:
            result_val = barrel_shift_left(op1, op2.range(4, 0));
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::SRL:
            result_val = barrel_shift_right(op1, op2.range(4, 0), false);
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::SRA:
            result_val = barrel_shift_right(op1, op2.range(4, 0), true);
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::XOR:
            result_val = op1 ^ op2;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::OR:
            result_val = op1 | op2;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::AND:
            result_val = op1 & op2;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::SLT:
            result_val = (sc_int<32>(op1) < sc_int<32>(op2)) ? 1 : 0;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::SLTU:
            result_val = (op1 < op2) ? 1 : 0;
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::MUL:
            result_val = multiply_32x32(op1, op2, true, true).range(31, 0);
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::MULH:
            result_val = multiply_32x32(op1, op2, true, true).range(63, 32);
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::MULHSU:
            result_val = multiply_32x32(op1, op2, true, false).range(63, 32);
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::MULHU:
            result_val = multiply_32x32(op1, op2, false, false).range(63, 32);
            valid.write(true);
            busy.write(false);
            break;
            
        case ALUOperation::DIV:
        case ALUOperation::DIVU:
        case ALUOperation::REM:
        case ALUOperation::REMU:
            // Start division for sequential operations
            start_division(op1, op2, (op == ALUOperation::DIV || op == ALUOperation::REM));
            valid.write(false);
            busy.write(true);
            break;
            
        default:
            result_val = 0;
            valid.write(false);
            busy.write(false);
            break;
    }
    
    result.write(result_val);
}

void ALU::sequential_operations() {
    if (reset.read()) {
        division_busy = false;
        division_cycles = 0;
        valid.write(false);
        busy.write(false);
        return;
    }
    
    if (division_busy) {
        step_division();
        division_cycles++;
        
        if (division_cycles >= 32) {
            // Division complete
            division_busy = false;
            division_cycles = 0;
            valid.write(true);
            busy.write(false);
        }
    }
}

sc_uint<32> ALU::barrel_shift_left(sc_uint<32> value, sc_uint<5> shift_amount) {
    if (shift_amount == 0) return value;
    
    sc_uint<32> result = value;
    for (int i = 0; i < 5; i++) {
        if (shift_amount[i]) {
            result = result << (1 << i);
        }
    }
    return result;
}

sc_uint<32> ALU::barrel_shift_right(sc_uint<32> value, sc_uint<5> shift_amount, bool arithmetic) {
    if (shift_amount == 0) return value;
    
    sc_uint<32> result = value;
    sc_uint<32> sign_bit = arithmetic ? value[31] : 0;
    
    for (int i = 0; i < 5; i++) {
        if (shift_amount[i]) {
            sc_uint<32> mask = (1 << (1 << i)) - 1;
            sc_uint<32> shifted = result >> (1 << i);
            if (arithmetic && sign_bit) {
                shifted |= ~mask;
            }
            result = shifted;
        }
    }
    return result;
}

sc_uint<64> ALU::multiply_32x32(sc_uint<32> a, sc_uint<32> b, bool signed_a, bool signed_b) {
    sc_int<64> signed_a_64 = signed_a ? sc_int<32>(a) : sc_uint<32>(a);
    sc_int<64> signed_b_64 = signed_b ? sc_int<32>(b) : sc_uint<32>(b);
    sc_int<64> product = signed_a_64 * signed_b_64;
    return sc_uint<64>(product);
}

void ALU::start_division(sc_uint<32> dividend_val, sc_uint<32> divisor_val, bool signed_div) {
    if (signed_div) {
        div_sign = (dividend_val[31] != divisor_val[31]) && (divisor_val != 0);
        dividend = dividend_val[31] ? -dividend_val : dividend_val;
        divisor = divisor_val[31] ? -divisor_val : divisor_val;
    } else {
        div_sign = false;
        dividend = dividend_val;
        divisor = divisor_val;
    }
    
    quotient = 0;
    remainder = dividend;
    division_busy = true;
    division_cycles = 0;
}

void ALU::step_division() {
    if (divisor <= remainder) {
        remainder = remainder - divisor;
        quotient = quotient | (1 << (31 - division_cycles));
    }
    divisor = divisor >> 1;
}

sc_uint<32> ALU::get_division_result(bool is_remainder) {
    if (is_remainder) {
        return div_sign ? -remainder : remainder;
    } else {
        return div_sign ? -quotient : quotient;
    }
}
