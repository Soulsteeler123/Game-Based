#include "cpu.h"
#include "../main.h"
#include "../BUS/bus.h"

//Returns condition of c and z flag
static bool condition(cpu_context *context) {
    //Automatically sets values of z and c flag
    bool z = CPU_ZFLAG;
    bool c = CPU_CFLAG;

    //Based on the instruction condition, returns if c or z flag is usable
    switch(context->inst.cond) {
        case CT_NONE: return true;
        case CT_C: return c;
        case CT_NC: return !c;
        case CT_Z: return z;
        case CT_NZ: return !z;
        default: return false;
    }
}

//Sets bits for flags
void cpu_flags(cpu_context *context, char z, char n, char h, char c) {
    if(z != -1) {
        BIT_SET(context->regs.f, 7, z);
    }
    if(n != -1) {
        BIT_SET(context->regs.f, 6, n);
    }
    if(h != -1) {
        BIT_SET(context->regs.f, 5, h);
    }
    if(c != -1) {
        BIT_SET(context->regs.f, 4, c);
    }
}

//Hit if an instruction is invalid
static void proc_none(cpu_context *context) {
    std::cout << "Error. Instruction is invalid." << std::endl;
    exit(-1);
}

static void proc_nop(cpu_context *context) {
    //Doesn't do anything, no operation
}

//Loads instruction
static void proc_ld(cpu_context *context) {
    //If a register is memory...
    if(context->dest_is_mem) {
        //if register 2 is a any combination of registers has to increment cycle
        if(context->inst.reg_2 >= RT_AF) {
            cycles(1);
            //Writes to bus (16 bit)
            bus_write16(context->mem, context->data);
        }
        else {
            //Writes to bus
            bus_write(context->mem, context->data);
        }
    }
    //If instruction mode is of AM_HL_SPR, special case
    else if(context->inst.mode == AM_HL_SPR) {
        //Determines hflag from register 2 (Everything but first bit) added with the data (everything but first bit) and if its greater than 16
        unsigned char hflag = (read_reg(context->inst.reg_2) & 0xF) + (context->data & 0xF) >= 0x10;
        //Determines hflag from register 2 (Everything but first two bits) added with the data (Everything but first two bits) and if its greater than 256
        unsigned char cflag = (read_reg(context->inst.reg_2) & 0xFF) + (context->data & 0xFF) >= 0x100;
        //Sets flags
        cpu_flags(context, 0, 0, hflag, cflag);
        //Sets registers (with register value of reg 1 being register 2 plus the context data)
        set_reg(context->inst.reg_1, read_reg(context->inst.reg_2) + (char)context->data);
    }
    else
        //Sets register 1 value
        set_reg(context->inst.reg_1, context->data);
}

//Emulated "goto" for the addressing
static void proc_goto(cpu_context *context, unsigned short address, bool push) {
    //Based on c or z flag...
    if(condition(context)) {
        //If a stack is needed, pushes pc register onto stack
        if(push) {
            stack_push16(context->regs.pc);
            cycles(2);
        }
        //Assigns pc register to address
        context->regs.pc = address;
        cycles(1);
    }
}

//Emulated "jump" for the data, doesn't need to use the stack
static void proc_jp(cpu_context *context) {
    proc_goto(context, context->data, false);
}

//Emulated "call" for the data, requires the use of the stack
static void proc_call(cpu_context *context) {
    proc_goto(context, context->data, true);
}

//Emulated "rest" for the data, requires the use of the stack
static void proc_rst(cpu_context *context) {
    proc_goto(context, context->inst.param, true);
}

//Emulated "return" for the stack
static void proc_ret(cpu_context *context) {
    //If there is a condition, increments cycles
    if(context->inst.cond != CT_NONE)
        cycles(1);
    //Based on condition...
    if(condition(context)) {
        //Gets low and high of the stack
        unsigned short lo = stack_pop();
        unsigned short hi = stack_pop();
        //Gets the full value of the original pc count and assigns it
        context->regs.pc = ((hi << 8) | lo);
        cycles(3);
    }
}

static void proc_reti(cpu_context *context) {
    //Same thing as return except enables master
    context->master_enabled = true;
    proc_ret(context);
}

static void proc_jr(cpu_context *context) {
    //Gets everything but first two bits from data
    char value = (char)(context->data & 0xFF);
    //Performs goto with the address being the pc register plus the rest of data, doesn't require the stack
    proc_goto(context, (unsigned short)(context->regs.pc + value), false);
}

static void proc_di(cpu_context *context) {
    //Just enables master
    context->master_enabled = false;
}

static void proc_ldh(cpu_context *context) {
    //If register 1 is register a
    if(context->inst.reg_1 == RT_A) 
        //Sets register 1 the bus read value of data or'd with FF
        set_reg(context->inst.reg_1, bus_read(0xFF00 | context->data));
    else
        //Writes to bus the data or'd with FF with register a
        bus_write(0xFF00 | context->data, context->regs.a);
        
    cycles(1);    
    
}

static void proc_xor(cpu_context *context) {
    //XOR's register a with the data anded with FF
    context->regs.a ^= context->data & 0xFF;
    //Sets the flags with the new register a
    cpu_flags(context, context->regs.a, 0, 0, 0);
}

static void proc_pop(cpu_context *context) {
    //Gets low and high values from stack
    unsigned short lo = stack_pop();
    unsigned short hi = stack_pop();
    //If register 1 is the AF register
    if(context->inst.reg_1 == RT_AF) 
        //Sets register 1 with the full value anded with FFF
        set_reg(context->inst.reg_1, ((hi << 8) | lo) & 0xFFF0);
    else
        //Else just sets register 1 with the full value
        set_reg(context->inst.reg_1, ((hi << 8) | lo));
    cycles(2);
}

static void proc_push(cpu_context *context) {
    //pushes the high value of register 1
    stack_push((read_reg(context->inst.reg_1) >> 8) & 0xFF);
    //pushes the low value of register 1
    stack_push((read_reg(context->inst.reg_1)) & 0xFF);
    cycles(3);
}