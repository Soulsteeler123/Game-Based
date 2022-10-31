#include "processors.h"
#include "../BUS/bus.h"
#include "../debug/debug.h"
#include "../Timer/timer.h"
//Current context information for cpu
cpu_context context = {0};

//Initializes default values for cpu
void cpu_init() {
    context.regs.pc = 0x100;
    context.regs.sp = 0xFFFE;
    *((short *)&context.regs.a) = 0xB001;
    *((short *)&context.regs.b) = 0x1300;
    *((short *)&context.regs.d) = 0xD800;
    *((short *)&context.regs.h) = 0x4D01;
    context.ie_reg = 0;
    context.iflags = 0;
    context.master_enabled = false;
    context.ime_enabled = false;
    get_timer()->div = 0xABCC;
}

void get_data() {
    //Sets default values for the memory and if the destination is memory
    context.mem = 0;
    context.dest_is_mem = false;
    //Determines work based on instruction mode
    switch(context.inst.mode) {
        //Nothing
        case AM_IMP: break;
        //Reads data from register 1
        case AM_R: context.data = read_reg(context.inst.reg_1); break;
        //Reads data from register 2
        case AM_R_R: context.data = read_reg(context.inst.reg_2); break;
        //Reads data from the bus based on the pc counter
        case AM_R_D8: context.data = bus_read(context.regs.pc++); cycles(1); break;
        //Gets 16 bit data from the bus based on pc counter
        case AM_R_D16:
        case AM_D16: {
            unsigned short lo = bus_read(context.regs.pc++);
            cycles(1);
            unsigned short hi = bus_read(context.regs.pc++);
            cycles(1);
            context.data = lo | (hi << 8);
            break;
        }
        case AM_MR_R: {
            //Reads data from register 2
            context.data = read_reg(context.inst.reg_2);
            //Reads the memory from register 1
            context.mem = read_reg(context.inst.reg_1);
            //Sets that the destination is memory to true
            context.dest_is_mem = true;

            //If register 1 is the C register, only gets first two bits
            if(context.inst.reg_1 == RT_C) {
                context.mem |= 0xFF00;
            }
            break;
        }
        case AM_R_MR: {
            //Reads address to read from register 2
            unsigned short address = read_reg(context.inst.reg_2);
            //If register 1 is C register, only gets first two bits
            if(context.inst.reg_1 == RT_C) {
                address |= 0xFF00;
            }

            //Gets data from address
            context.data = bus_read(address);
            cycles(1);
            break;
        }
        case AM_R_HLI: {
            //Reads data from bus based on register data from register 2
            context.data = bus_read(read_reg(context.inst.reg_2));
            cycles(1);
            //Adds 1 to the HL register
            set_reg(RT_HL, read_reg(RT_HL) + 1);
            break;
        }
        case AM_R_HLD: {
            //Reads data from bus based on register data from register 2
            context.data = bus_read(read_reg(context.inst.reg_2));
            cycles(1);
            //Subtracts 1 from the HL register
            set_reg(RT_HL, read_reg(RT_HL) - 1);
            break;
        }
        case AM_HLI_R: {
            //Gets data from register 2
            context.data = read_reg(context.inst.reg_2);
            //Gets memory from register 1
            context.mem = read_reg(context.inst.reg_1);
            //Determines that the destination is memory
            context.dest_is_mem = true;
            //Adds 1 to the HL register
            set_reg(RT_HL, read_reg(RT_HL) + 1);
            break;
        }
        case AM_HLD_R: {
            //Gets data from register 2
            context.data = read_reg(context.inst.reg_2);
            //Gets memory from register 1
            context.mem = read_reg(context.inst.reg_1);
            //Determines that the destination is memory
            context.dest_is_mem = true;
            //Subtracts 1 from the HL register
            set_reg(RT_HL, read_reg(RT_HL) - 1);
            break;
        }
        case AM_D8:
        case AM_HL_SPR:
        case AM_R_A8: {
            //Reads data from bus based on pc counter
            context.data = bus_read(context.regs.pc++);
            cycles(1);
            break;
        }
        case AM_A8_R: {
            //Reads first two bits of memory from the bus based on pc counter
            context.mem = bus_read(context.regs.pc++) | 0xFF00;
            //Determines destination is memory
            context.dest_is_mem = true;
            cycles(1);
            break;
        }
        case AM_A16_R:
        case AM_D16_R: {
            //Reads low and high bits from the bus
            unsigned short lo = bus_read(context.regs.pc++);
            cycles(1);
            unsigned short hi = bus_read(context.regs.pc++);
            cycles(1);
            //Sets memory to be low or'd with high shifted to the left by 8
            context.mem = lo | (hi << 8);
            //Determines destination is memory
            context.dest_is_mem = true;
            //Reads data from register 2
            context.data = read_reg(context.inst.reg_2);
            break;
        }
        case AM_MR_D8: {
            //Reads data from bus based on pc counter
            context.data = bus_read(context.regs.pc++);
            cycles(1);
            //Reads memory from register 1
            context.mem = read_reg(context.inst.reg_1);
            //Determines destination is memory
            context.dest_is_mem = true;
            break;
        }
        case AM_MR: {
            //Reads memory from register 1
            context.mem = read_reg(context.inst.reg_1);
            //Determines destination is memory
            context.dest_is_mem = true;
            //Reads data from bus from register 1
            context.data = bus_read(read_reg(context.inst.reg_1));
            cycles(1);
            break;
        }
         case AM_R_A16: {
            //Assigns low and high bits based on bus
            unsigned short lo = bus_read(context.regs.pc++);
            cycles(1);
            unsigned short hi = bus_read(context.regs.pc++);
            cycles(1);
            //Gets address from low and high
            unsigned short address = lo | (hi << 8);
            //Reads data from bus
            context.data = bus_read(address);
            cycles(1);
            break;
        }
    }
}

void get_instr() {
    //Gets opcode from bus based on pc counter
    context.opcode = bus_read(context.regs.pc++);
    //Gets instruction based on opcode
    context.inst = get_instruction(context.opcode);
    //Throws error if the instruction type is not yet implemented
    if(context.inst.type == IN_NONE) {
        std::cout << "Unknown instruction. ERROR. Opcode: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.opcode << std::endl;
        exit(-1);
    }
}

void exec() {
    //Based on instruction type, performs an instruction
    switch(context.inst.type) {
        case IN_NONE: proc_none(&context); break;
        case IN_NOP: proc_nop(&context); break;
        case IN_LD: proc_ld(&context); break;
        case IN_JP: proc_jp(&context); break;
        case IN_CALL: proc_call(&context); break;
        case IN_RST: proc_rst(&context); break;
        case IN_RET: proc_ret(&context); break;
        case IN_RETI: proc_reti(&context); break;
        case IN_JR: proc_jr(&context); break;
        case IN_DI: proc_di(&context); break;
        case IN_EI: proc_ei(&context); break;
        case IN_LDH: proc_ldh(&context); break;
        case IN_XOR: proc_xor(&context); break;
        case IN_PUSH: proc_push(&context); break;
        case IN_POP: proc_pop(&context); break;
        case IN_INC: proc_inc(&context); break;
        case IN_DEC: proc_dec(&context); break;
        case IN_ADD: proc_add(&context); break;
        case IN_ADC: proc_adc(&context); break;
        case IN_SUB: proc_sub(&context); break;
        case IN_SBC: proc_sbc(&context); break;
        case IN_AND: proc_and(&context); break;
        case IN_OR: proc_or(&context); break;
        case IN_CP: proc_cp(&context); break;
        case IN_CB: proc_cb(&context); break;
        case IN_RLCA: proc_rlca(&context); break;
        case IN_RRCA: proc_rrca(&context); break;
        case IN_RLA: proc_rla(&context); break;
        case IN_RRA: proc_rra(&context); break;
        case IN_STOP: proc_stop(&context); break;
        case IN_DAA: proc_daa(&context); break;
        case IN_CPL: proc_cpl(&context); break;
        case IN_SCF: proc_scf(&context); break;
        case IN_CCF: proc_ccf(&context); break;
        case IN_HALT: proc_halt(&context); break;
        default: NOT_IMPL;
    }
}

bool cpu_step() {
    //As long as the cpu hasn't halted
    std::ofstream file;
    //If context is still in motion
    if(!context.halt) {
        file.open("output.txt", std::ios_base::app);
        unsigned short temp_pc = context.regs.pc;
        //Gets the instruction
        get_instr();
        cycles(1);
        //Gets the data
        get_data();
        /*
        //Outputs debug information
        std::string flags = ((context.regs.f & (1 << 7)) ? "Z" : "-");
        flags += ((context.regs.f & (1 << 6)) ? "N" : "-");
        flags += ((context.regs.f & (1 << 5)) ? "H" : "-");
        flags += ((context.regs.f & (1 << 4)) ? "C" : "-");
        std::cout << "Ticks: " << std::setfill('0') << std::setw(8) << emu_get_struct()->ticks;
        std::cout << "\tPC: " << std::setfill('0') << std::setw(4) << std::hex << (int)temp_pc << "\t" << get_name(context.inst.type);
        std::cout << "\tExecuting instruction: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.opcode;
        std::cout << "\tNext two opcodes: (" << std::setfill('0') << std::setw(2) << std::hex << (int)bus_read(temp_pc + 1) << ", " << std::setw(2) << (int)bus_read(temp_pc + 2) << ").";
        std::cout << "\tRegister A: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.regs.a << " Register BC: " << std::setw(2) << (int)context.regs.b << std::setw(2) << (int)context.regs.c 
        << " Register DE: " << std::setw(2) << (int)context.regs.d << std::setw(2) << (int)context.regs.e << " Register HL: " << std::setw(2) << (int)context.regs.h << 
        std::setw(2) << (int)context.regs.l;
        std::cout << "\tFlags: " << flags << std::endl;
        file << "Ticks: " << std::setfill('0') << std::setw(8) << emu_get_struct()->ticks <<
        "\tPC: " << std::setfill('0') << std::setw(4) << std::hex << (int)temp_pc << "\t" << get_name(context.inst.type) <<
        "\tExecuting instruction: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.opcode <<
        "\tNext two opcodes: (" << std::setfill('0') << std::setw(2) << std::hex << (int)bus_read(temp_pc + 1) << ", " << std::setw(2) << (int)bus_read(temp_pc + 2) << ")." <<
        "\tRegister A: " << std::setfill('0') << std::setw(2) << std::hex << (int)context.regs.a << " Register BC: " << std::setw(2) << (int)context.regs.b << std::setw(2) << (int)context.regs.c <<
        " Register DE: " << std::setw(2) << (int)context.regs.d << std::setw(2) << (int)context.regs.e << " Register HL: " << std::setw(2) << (int)context.regs.h <<  std::setw(2) << (int)context.regs.l <<
        "\tFlags: " << flags << std::endl;
        
        //Attempts to grab debug information from IO and output
        debug_update();
        debug_print();
        */
        //Executes the instruction
        exec();
        file.close();
    }
    else {
        //If the interrupt flags are set, exits the halt status
        cycles(1);
        if(context.iflags)
            context.halt = false;
    }

    //If master is enabled, handles the interrupt and unsets the ime
    if(context.master_enabled) {
        handle_interrupt();
        context.ime_enabled = false;
    }

    //If ime is enabled, enables master
    if(context.ime_enabled)
        context.master_enabled = true;
    return true;
}

unsigned short read_reg(reg_type reg) {
    //Returns value of register
    switch(reg) {
        case RT_A: return context.regs.a;
        case RT_F: return context.regs.f;
        case RT_B: return context.regs.b;
        case RT_C: return context.regs.c;
        case RT_D: return context.regs.d;
        case RT_E: return context.regs.e;
        case RT_H: return context.regs.h;
        case RT_L: return context.regs.l;       
        case RT_AF: return rev_reg(*(unsigned short *)&context.regs.a);
        case RT_BC: return rev_reg(*(unsigned short *)&context.regs.b);
        case RT_DE: return rev_reg(*(unsigned short *)&context.regs.d);
        case RT_HL: return rev_reg(*(unsigned short *)&context.regs.h);
        
        case RT_PC: return context.regs.pc;
        case RT_SP: return context.regs.sp;
        default: return 0;
    }
}

unsigned short rev_reg(unsigned short reg) {
    //Reverses low and high bits of register
    return ((reg & 0xFF00) >> 8) | ((reg & 0x00FF) << 8);
}

void set_reg(reg_type reg, unsigned short value) {
    //Sets register to value
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

//Returns 8 bit value of registers
unsigned char read_reg8(reg_type reg) {
    switch(reg) {
        case RT_A: return context.regs.a;
        case RT_F: return context.regs.f;
        case RT_B: return context.regs.b;
        case RT_C: return context.regs.c;
        case RT_D: return context.regs.d;
        case RT_E: return context.regs.e;
        case RT_H: return context.regs.h;
        case RT_L: return context.regs.l;
        case RT_HL: return bus_read(read_reg(RT_HL));
        default: std::cout << "Error. Invalid reg8." << std::endl; NOT_IMPL
    }
}

//Sets 8 bit value to registers
void set_reg8(reg_type reg, unsigned char value) {
    switch(reg) {
        case RT_A: context.regs.a = value & 0xFF; break;
        case RT_F: context.regs.f = value & 0xFF; break;
        case RT_B: context.regs.b = value & 0xFF; break;
        case RT_C: context.regs.c = value & 0xFF; break;
        case RT_D: context.regs.d = value & 0xFF; break;
        case RT_E: context.regs.e = value & 0xFF; break;
        case RT_H: context.regs.h = value & 0xFF; break;
        case RT_L: context.regs.l = value & 0xFF; break;
        case RT_HL: bus_write(read_reg(RT_HL), value); break;
        default: std::cout << "Error. Invalid reg8." << std::endl; NOT_IMPL
    }
}

unsigned char get_ie_reg() {
    //Returns ie register
    return context.ie_reg;
}

void set_ie_reg(unsigned char value) {
    //Sets ie register to value
    context.ie_reg = value;
}

void stack_push(unsigned char value) {
    //sp counter used to keep track of stack, decrements when adding
    context.regs.sp--;
    //"Pushes onto the stack"
    bus_write(context.regs.sp, value);
}

void stack_push16(unsigned short value) {
    //Pushes split value onto the stack
    stack_push((value >> 8) & 0xFF);
    stack_push(value & 0xFF);
}

unsigned char stack_pop() {
    //Returns the "top of stack" and adds to counter to keep positioning
    return(bus_read(context.regs.sp++));
}

unsigned short stack_pop16() {
    //Reads high and low of value and returns it
    return((stack_pop() << 8) | stack_pop());
}

unsigned char get_iflags() {
    return context.iflags;
}

void set_iflags(unsigned char value) {
    context.iflags = value;
}

//Handles the interrupts by pushing pc value onto the stack and reassigning the pc
void int_handle(unsigned short address) {
    stack_push16(context.regs.pc);
    context.regs.pc = address;
}

bool int_check(unsigned short address, interrupt_type inter) {
    //Checks if an interrupt flag is set
    if((context.iflags & inter) && (context.ie_reg & inter)) {
        //Handles the interrupt
        int_handle(address);
        //Ands its flag values with the given interrupt type's approximate value
        context.iflags &= ~inter;
        //Exits the halt
        context.halt = false;
        //Disables master
        context.master_enabled = false;

        return true;
    }
    else
        return false;
}

//Or's the current flags with the interrupt type
void request_interrupt(interrupt_type type) {
    context.iflags |= type;
}

void handle_interrupt() {
    //Checks which interrupt needs to be handled and handles it
    if(int_check(0x40, VBLANK)) {

    }
    else if(int_check(0x48, LCD_STAT)) {

    }
    else if(int_check(0x50, TIMER)) {
        
    }
    else if(int_check(0x58, SERIAL)) {
        
    }
    else if(int_check(0x60, JOYPAD)) {
        
    }
}