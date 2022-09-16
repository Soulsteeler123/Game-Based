#include "instr.h"
#include "../CPU/cpu.h"

//Lookup table for all instruction registers based on opcode
static const std::map<int, instruction> instructions {
    {-1, {IN_NONE}},
    {0x00, {IN_NOP, AM_IMP}},
    {0x05, {IN_DEC, AM_R, RT_B}},
    {0x0E, {IN_LD, AM_R_D8, RT_C}},
    {0xAF, {IN_XOR, AM_R, RT_A}},
    {0xC3, {IN_JP, AM_D16}},
    {0xF3, {IN_DI}}
};

//Returns instructions based on opcode, if opcode is invalid it means instruction is either invalid or not yet implemented
instruction get_instruction(unsigned char opcode) {
    try {
        return instructions.at(opcode);
    } catch(...) {
        return(instructions.at(-1));
    }
}

//Lookup table for instruction action type
std::string instruct_lookup[] = {
    "<NONE>",
    "NOP",
    "LD",
    "INC",
    "DEC",
    "RLCA",
    "ADD",
    "RRCA",
    "STOP",
    "RLA",
    "JR",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF",
    "HALT",
    "ADC",
    "SUB",
    "SBC",
    "AND",
    "XOR",
    "OR",
    "CP",
    "POP",
    "JP",
    "PUSH",
    "RET",
    "CB",
    "CALL",
    "RETI",
    "LDH",
    "JPHL",
    "DI",
    "EI",
    "RST",
    "IN_ERR",
    "IN_RLC", 
    "IN_RRC",
    "IN_RL", 
    "IN_RR",
    "IN_SLA", 
    "IN_SRA",
    "IN_SWAP", 
    "IN_SRL",
    "IN_BIT", 
    "IN_RES", 
    "IN_SET"
};

//Returns name of instruction action
std::string get_name(in_type type) {
    return instruct_lookup[type];
}