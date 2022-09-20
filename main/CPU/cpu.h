#pragma once
#include "../reusable.h"
//Causes get_instruction to be overloaded for some reason?
#include "../instructions/instr.h"

#define CPU_ZFLAG BIT(context->regs.f, 7)
#define CPU_CFLAG BIT(context->regs.f, 4)

//Defines all registers
struct registers {
    unsigned char a;
    unsigned char f;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    unsigned char e;
    unsigned char h;
    unsigned char l;
    unsigned short pc;
    unsigned short sp;
};

//Defines all members of the CPU
struct cpu_context {
    registers regs;
    unsigned short data;
    unsigned short mem;
    unsigned char opcode;
    instruction inst;
    bool stopped;
    bool step;
    bool dest_is_mem;
    bool master_enabled;
};

//Initalizes the CPU
void cpu_init();
//Checks to make sure CPU was able to step properly
bool cpu_step();
//Retrieves current instruction
void get_instr();
//Gets the current data
void get_data();
//Executes CPU
void exec();
//Returns register based on register type
unsigned short read_reg(reg_type reg);
//Reverses bytes of register
unsigned short rev_reg(unsigned short reg);