#pragma once
#include "../reusable.h"
#include "../instructions/instr.h"

//Short cuts to deal with z and c flag bit sets
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
    unsigned char ie_reg;
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
//Sets the register to a given 16 bit value
void set_reg(reg_type reg, unsigned short value);

unsigned char read_reg8(reg_type reg);
void set_reg8(reg_type reg, unsigned char value);

//Returns 8 bit value in the ie register
unsigned char get_ie_reg();
//Sets the ie register to an 8 bit value
void set_ie_reg(unsigned char value);

//Pushs values for the bus and pushes it on top of the "stack"
void stack_push(unsigned char value);
//Pushs values for the bus and pushes it on top of the "stack" (16 bit)
void stack_push16(unsigned short value);

//Pops top value of the "stack"
unsigned char stack_pop();
//Pops top value of the "stack" (16 bit)
unsigned short stack_pop16();