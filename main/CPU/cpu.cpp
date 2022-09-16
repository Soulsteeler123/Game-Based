#include "cpu.h"
#include "../BUS/bus.h"
#include "../main.h"

cpu_context context;

void cpu_init() {
    context.regs.pc = 0x100;
    context.stopped = false;
}

void get_data() {
    context.mem = 0;
    context.dest_is_mem = false;
    switch(context.inst.mode) {
        case AM_IMP: break;
        case AM_R: context.data = read_reg(context.inst.reg_1); break;
        case AM_R_D8: context.data = bus_read(context.regs.pc++); cycles(1); break;
        case AM_D16: {
            unsigned short lo = bus_read(context.regs.pc++);
            unsigned hi = bus_read(context.regs.pc++);
            context.data = lo | (hi << 8);
            cycles(2);
            break;
        }
    }
}

void get_instr() {
    context.opcode = bus_read(context.regs.pc++);
    context.inst = get_instruction(context.opcode);
    if(context.inst.type == IN_NONE) {
        std::cout << "Unknown instruction. ERROR. Opcode: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.opcode << std::endl;
        exit(-1);
    }
}

void exec() {
    printf("\tNot yet executing\n");
}

bool cpu_step() {
    if(!context.stopped) {
        std::cout << "Executing instruction: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.opcode;
        std::cout << "\tPC: " << std::setfill('0') << std::setw(4) << std::hex << (int)context.regs.pc << std::endl;
        get_instr();
        get_data();
        exec();
    }
    return true;
}

unsigned short read_reg(reg_type reg) {
    switch(reg) {
        case RT_A: return context.regs.a;
        case RT_F: return context.regs.f;
        case RT_B: return context.regs.b;
        case RT_C: return context.regs.c;
        case RT_D: return context.regs.d;
        case RT_E: return context.regs.e;
        case RT_H: return context.regs.h;
        case RT_L: return context.regs.l;

        case RT_AF: return rev_reg((unsigned short)context.regs.a);
        case RT_BC: return rev_reg((unsigned short)context.regs.b);
        case RT_DE: return rev_reg((unsigned short)context.regs.d);
        case RT_HL: return rev_reg((unsigned short)context.regs.h);

        case RT_PC: return context.regs.pc;
        case RT_SP: return context.regs.sp;
        default: return 0;
    }
}

unsigned short rev_reg(unsigned short reg) {
    return ((reg & 0xFF00) >> 8) | ((reg & 0x00FF) << 8);
}