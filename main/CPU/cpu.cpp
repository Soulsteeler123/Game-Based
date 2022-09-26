#include "processors.h"
#include "../BUS/bus.h"

cpu_context context;

void cpu_init() {
    context.regs.pc = 0x100;
    context.regs.a = 0x01;
    context.stopped = false;
}

void get_data() {
    context.mem = 0;
    context.dest_is_mem = false;
    switch(context.inst.mode) {
        case AM_IMP: break;
        case AM_R: context.data = read_reg(context.inst.reg_1); break;
        case AM_R_R: context.data = read_reg(context.inst.reg_2); break;
        case AM_R_D8: context.data = bus_read(context.regs.pc++); cycles(1); break;
        case AM_R_D16:
        case AM_D16: {
            unsigned short lo = bus_read(context.regs.pc++);
            unsigned short hi = bus_read(context.regs.pc++);
            context.data = lo | (hi << 8);
            cycles(2);
            break;
        }
        case AM_MR_R: {
            context.data = read_reg(context.inst.reg_2);
            context.mem = read_reg(context.inst.reg_1);
            context.dest_is_mem = true;

            if(context.inst.reg_1 == RT_C) {
                context.mem |= 0xFF00;
            }
            break;
        }
        case AM_R_MR: {
            unsigned short address = read_reg(context.inst.reg_2);

            if(context.inst.reg_1 == RT_C) {
                address |= 0xFF00;
            }

            context.data = bus_read(address);
            cycles(1);
            break;
        }
        case AM_R_HLI: {
            context.data = bus_read(read_reg(context.inst.reg_2));
            cycles(1);
            set_reg(RT_HL, read_reg(RT_HL) + 1);
            break;
        }
        case AM_R_HLD: {
            context.data = bus_read(read_reg(context.inst.reg_2));
            cycles(1);
            set_reg(RT_HL, read_reg(RT_HL) - 1);
            break;
        }
        case AM_HLI_R: {
            context.data = read_reg(context.inst.reg_2);
            context.mem = read_reg(context.inst.reg_1);
            context.dest_is_mem = true;
            set_reg(RT_HL, read_reg(RT_HL) + 1);
            break;
        }
        case AM_HLD_R: {
            context.data = read_reg(context.inst.reg_2);
            context.mem = read_reg(context.inst.reg_1);
            context.dest_is_mem = true;
            set_reg(RT_HL, read_reg(RT_HL) - 1);
            break;
        }
        case AM_R_A8: {
            context.data = bus_read(context.regs.pc++);
            cycles(1);
            break;
        }
        case AM_A8_R: {
            context.mem = bus_read(context.regs.pc++) | 0xFF00;
            context.dest_is_mem = true;
            cycles(1);
            break;
        }
        case AM_D8:
        case AM_HL_SPR: {
            context.data = bus_read(context.regs.pc++);
            cycles(1);
            break;
        }
        case AM_A16_R:
        case AM_D16_R: {
            unsigned short lo = bus_read(context.regs.pc++);
            unsigned short hi = bus_read(context.regs.pc++);
            context.mem = lo | (hi << 8);
            context.dest_is_mem = true;
            context.data = read_reg(context.inst.reg_2);
            cycles(2);
            break;
        }
        case AM_MR_D8: {
            context.data = bus_read(context.regs.pc++);
            cycles(1);
            context.mem = read_reg(context.inst.reg_1);
            context.dest_is_mem = true;
            break;
        }
        case AM_MR: {
            context.mem = read_reg(context.inst.reg_1);
            context.dest_is_mem = true;
            context.data = bus_read(read_reg(context.inst.reg_1));
            cycles(1);
            break;
        }
         case AM_R_A16: {
            unsigned short lo = bus_read(context.regs.pc++);
            unsigned short hi = bus_read(context.regs.pc++);
            unsigned short address = lo | (hi << 8);
            context.data = bus_read(address);
            cycles(3);
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

    switch(context.inst.type) {
        case IN_NONE: proc_none(&context); break;
        case IN_NOP: proc_nop(&context); break;
        case IN_LD: proc_ld(&context); break;
        case IN_JP: proc_jp(&context); break;
        case IN_DI: proc_di(&context); break;
        case IN_LDH: proc_ldh(&context); break;
        case IN_XOR: proc_xor(&context); break;
        default: NOT_IMPL;
    }
}

bool cpu_step() {
    if(!context.stopped) {
        unsigned short temp_pc = context.regs.pc;
        get_instr();
        get_data();
        std::cout << "PC: " << std::setfill('0') << std::setw(4) << std::hex << (int)temp_pc << "\t" << get_name(context.inst.type);
        std::cout << "\tExecuting instruction: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.opcode;
        std::cout << "\tNext two opcodes: (" << std::setfill('0') << std::setw(2) << std::hex << (int)bus_read(temp_pc + 1) << ", " << std::setw(2) << (int)bus_read(temp_pc + 2) << ").";
        std::cout << "\tRegister A: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.regs.a << " Register BC: " << std::setw(2) << (int)context.regs.b << std::setw(2) << (int)context.regs.c 
        << " Register DE: " << std::setw(2) << (int)context.regs.d << std::setw(2) << (int)context.regs.e << " Register HL: " << std::setw(2) << (int)context.regs.h << 
        std::setw(2) << (int)context.regs.l << std::endl;
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

void set_reg(reg_type reg, unsigned short value) {
    switch(reg) {
        case RT_A: context.regs.a = value & 0xFF; break;
        case RT_F: context.regs.f = value & 0xFF; break;
        case RT_B: context.regs.b = value & 0xFF; break;
        case RT_C: context.regs.c = value & 0xFF; break;
        case RT_D: context.regs.d = value & 0xFF; break;
        case RT_E: context.regs.e = value & 0xFF; break;
        case RT_H: context.regs.h = value & 0xFF; break;
        case RT_L: context.regs.l = value & 0xFF; break;

        case RT_AF: *((unsigned short *)&context.regs.a) = rev_reg(value); break;
        case RT_BC: *((unsigned short *)&context.regs.b) = rev_reg(value); break;
        case RT_DE: *((unsigned short *)&context.regs.d) = rev_reg(value); break;
        case RT_HL: *((unsigned short *)&context.regs.h) = rev_reg(value); break;

        case RT_PC: context.regs.pc = value;
        case RT_SP: context.regs.sp = value;
        default: break;
    }
}

unsigned char get_ie_reg() {
    return context.ie_reg;
}
void set_ie_reg(unsigned char value) {
    context.ie_reg = value;
}