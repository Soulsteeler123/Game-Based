#include "cpu.h"
#include "../main.h"

static bool condition(cpu_context *context) {
    bool z = CPU_ZFLAG;
    bool c = CPU_CFLAG;

    switch(context->inst.cond) {
        case CT_NONE: return true;
        case CT_C: return c;
        case CT_NC: return !c;
        case CT_Z: return z;
        case CT_NZ: return !z;
        default: return false;
    }
}

static void proc_none(cpu_context *context) {
    std::cout << "Error. Instruction is invalid." << std::endl;
    exit(-1);
}

static void proc_nop(cpu_context *context) {
    //Doesn't do anything, no operation
}

static void proc_ld(cpu_context *context) {
    //Not implemented
}

static void proc_jp(cpu_context *context) {
    if(condition(context)) {
        context->regs.pc = context->data;
        cycles(1);
    }
}

static void proc_di(cpu_context *context) {
    context->master_enabled = false;
}

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

static void proc_xor(cpu_context *context) {
    context->regs.a ^= context->data & 0xFF;
    cpu_flags(context, context->regs.a, 0, 0, 0);
}