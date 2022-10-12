#include "cpu.h"
#include "../main.h"
#include "../BUS/bus.h"

reg_type cb_lookup[] = {
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_HL,
    RT_A
};

reg_type cb_decode(unsigned char reg) {
    if(reg > 0b111)
        return RT_NONE;
    else
        return cb_lookup[reg];
}

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

static bool is_16bit(reg_type reg) {
    return reg >= RT_AF;
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
        if(is_16bit(context->inst.reg_2)) {
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
        cycles(1);
        unsigned short hi = stack_pop();
        cycles(1);
        //Gets the full value of the original pc count and assigns it
        context->regs.pc = ((hi << 8) | lo);
        cycles(1);
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

static void proc_ei(cpu_context *context) {
    //Just enables master
    context->ime_enabled = true;
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
    cpu_flags(context, context->regs.a == 0, 0, 0, 0);
}

static void proc_pop(cpu_context *context) {
    //Gets low and high values from stack
    unsigned short lo = stack_pop();
    cycles(1);
    unsigned short hi = stack_pop();
    cycles(1);
    //If register 1 is the AF register
    if(context->inst.reg_1 == RT_AF) 
        //Sets register 1 with the full value anded with FFF
        set_reg(context->inst.reg_1, ((hi << 8) | lo) & 0xFFF0);
    else
        //Else just sets register 1 with the full value
        set_reg(context->inst.reg_1, ((hi << 8) | lo));
}

static void proc_push(cpu_context *context) {
    cycles(1);
    //pushes the high value of register 1
    stack_push((read_reg(context->inst.reg_1) >> 8) & 0xFF);
    cycles(1);
    //pushes the low value of register 1
    stack_push((read_reg(context->inst.reg_1)) & 0xFF);
    cycles(1);
}

static void proc_inc(cpu_context *context) {
    unsigned short value = read_reg(context->inst.reg_1) + 1;
    if(is_16bit(context->inst.reg_1))
        cycles(1);
    if(context->inst.reg_1 == RT_HL && context->inst.mode == AM_MR) {
        value = bus_read(read_reg(RT_HL)) + 1;
        value &= 0xFF;
        bus_write(read_reg(RT_HL), value);
    }
    else {
        set_reg(context->inst.reg_1, value);
        value = read_reg(context->inst.reg_1);
    }

    //If bottom two bits are not set
    if((context->opcode & 0x03) != 0x03) 
        cpu_flags(context, value == 0, 0, (value & 0x0F) == 0, -1);
    
}

static void proc_dec(cpu_context *context) {
    unsigned short value = read_reg(context->inst.reg_1) - 1;

    if(is_16bit(context->inst.reg_1))
        cycles(1);
    if(context->inst.reg_1 == RT_HL && context->inst.mode == AM_MR) {
        value = bus_read(read_reg(RT_HL)) - 1;
        bus_write(read_reg(RT_HL), value);
    }
    else {
        set_reg(context->inst.reg_1, value);
        value = read_reg(context->inst.reg_1);
    }

    //If bottom two bits are not set
    if((context->opcode & 0x0B) != 0x0B) 
        cpu_flags(context, value == 0, 1, (value & 0x0F) == 0x0F, -1);
}

static void proc_add(cpu_context *context) {
    unsigned int value = read_reg(context->inst.reg_1) + context->data;
    bool is16 = is_16bit(context->inst.reg_1);
    if(is16)
        cycles(1);
    
    if(context->inst.reg_1 == RT_SP) 
        value = read_reg(context->inst.reg_1) + (char)context->data;
    int z, h, c;
    if(is16) {
        z = -1;
        h = (read_reg(context->inst.reg_1) & 0xFFF) + (context->data & 0xFFF) >= 0x1000;
        c = ((unsigned int)read_reg(context->inst.reg_1)) + ((unsigned int)context->data) >= 0x10000;
    }
    else if(context->inst.reg_1 == RT_SP) {
        z = 0;
        h = (read_reg(context->inst.reg_1) & 0xF) + (context->data & 0xF) >= 0x10;
        c = (int)(read_reg(context->inst.reg_1) & 0xFF) + (int)(context->data & 0xFF) >= 0x100;
    }
    else {
        z = (value & 0xFF) == 0;
        h = (read_reg(context->inst.reg_1) & 0xF) + (context->data & 0xF) >= 0x10;
        c = (int)(read_reg(context->inst.reg_1) & 0xFF) + (int)(context->data & 0xFF) >= 0x100;
    }

    set_reg(context->inst.reg_1, value & 0xFFFF);
    cpu_flags(context, z, 0, h, c);
}

//Add with carry
static void proc_adc(cpu_context *context) {
    unsigned short data = context->data;
    unsigned short rega = context->regs.a;
    unsigned short flagc = CPU_CFLAG;

    context->regs.a = (data, rega, flagc) & 0xFF;
    cpu_flags(context, context->regs.a == 0, 0, (rega & 0xF) + (data & 0xF) + flagc > 0xF, data + rega + flagc > 0xFF);
}

static void proc_sub(cpu_context *context) {
    unsigned short value = read_reg(context->inst.reg_1) - context->data;
    int z = value == 0;
    int h = ((int)read_reg(context->inst.reg_1) & 0xF) - ((int)context->data & 0xF) < 0;
    int c = ((int)read_reg(context->inst.reg_1)) - ((int)context->data) < 0;

    set_reg(context->inst.reg_1, value);
    cpu_flags(context, z, 1, h, c);
}

//Subtract with carry
static void proc_sbc(cpu_context *context) {
    unsigned char value = context->data + CPU_CFLAG;
    int z = read_reg(context->inst.reg_1) - value == 0;
    int h = ((int)read_reg(context->inst.reg_1) & 0xF) - ((int)context->data & 0xF) - (int)CPU_CFLAG < 0;
    int c = ((int)read_reg(context->inst.reg_1)) - ((int)context->data) - (int)CPU_CFLAG < 0;

    set_reg(context->inst.reg_1, read_reg(context->inst.reg_1) - value);
    cpu_flags(context, z, 1, h, c);
}

static void proc_and(cpu_context *context) {
    context->regs.a &= context->data;
    cpu_flags(context, context->regs.a == 0, 0, 1, 0);
}

static void proc_or(cpu_context *context) {
    context->regs.a |= context->data & 0xFF;
    cpu_flags(context, context->regs.a == 0, 0, 0, 0);
}

static void proc_cp(cpu_context *context) {
    int value = (int)context->regs.a - (int)context->data;
    cpu_flags(context, value == 0, 1, ((int)context->regs.a & 0x0F) - ((int)context->data & 0x0F) < 0, value < 0);
}

static void proc_cb(cpu_context *context) {
    unsigned op = context->data;
    reg_type reg = cb_decode(op & 0b111);
    unsigned char bit = (op >> 3) & 0b111;
    unsigned char bitop = (op >> 6) & 0b11;
    unsigned char regval = read_reg8(reg);
    if(reg == RT_HL)
        cycles(2);

    switch(bitop) {
        case 1:
            //BIT
            cpu_flags(context, !(regval & (1 << bit)), 0, 1, -1); return;
        case 2:
            //RST
            set_reg8(reg, (regval &= ~(1 << bit))); return;
        case 3:
            //SET
            set_reg8(reg, (regval |= (1 << bit))); return;
    }

    bool cflag = CPU_CFLAG;

    switch(bit) {
        case 0: {
            //RLC
            bool setC = false;
            unsigned char result = (regval << 1) & 0xFF;

            if((regval & (1 << 7)) != 0) {
                result |= 1;
                setC = true;
            }

            set_reg8(reg, result);
            cpu_flags(context, result == 0, 0, 0, setC);
            break;
        }
        case 1: {
            //RRC
            unsigned char old = regval;
            regval >>= 1;
            regval |= (old << 7);
            set_reg8(reg, regval);
            cpu_flags(context, !regval, 0, 0, old & 1);
            break;
        }
        case 2: {
            //RL
            unsigned char old = regval;
            regval >>= 1;
            regval |= cflag;
            set_reg8(reg, regval);
            cpu_flags(context, !regval, 0, 0, !!(old & 0x80));
            break;
        }
        case 3: {
            //RR
            unsigned char old = regval;
            regval >>= 1;
            regval |= (cflag << 7);
            set_reg8(reg, regval);
            cpu_flags(context, !regval, 0, 0, old & 1);
            break;
        }
        case 4: {
            //SLA
            unsigned char old = regval;
            regval <<= 1;

            set_reg8(reg, regval);
            cpu_flags(context, !regval, 0, 0, !!(old & 0x80));
            break;
        }
        case 5: {
            //SRA
            unsigned char value = (int8_t)regval >> 1;
            set_reg8(reg, value);
            cpu_flags(context, !value, 0, 0, regval & 1);
            break;
        }
        case 6: {
            //SWAP
            regval = ((regval & 0xF0) >> 4) | ((regval & 0xF) << 4);
            set_reg8(reg, regval);
            cpu_flags(context, regval == 0, 0, 0, 0);
            break;
        }
        case 7: {
            //SRL
            unsigned char value = regval >> 1;
            set_reg8(reg, value);
            cpu_flags(context, !value, 0, 0, regval & 1);
            break;
        }
        default: std::cout << "ERROR. Invalid CB: " << std::setfill('0') << std::setw(2) << std::hex << (int)op << std::endl; NOT_IMPL
    }
}

static void proc_rlca(cpu_context *context) {
    unsigned char temp = context->regs.a;
    bool cset = (temp >> 7) & 1;
    temp = (temp >> 1) | cset;
    context->regs.a = temp;

    cpu_flags(context, 0, 0, 0, cset);
}

static void proc_rrca(cpu_context *context) {
    unsigned char cset = context->regs.a & 1;
    context->regs.a >>= 1;
    context->regs.a |= (cset << 7);

    cpu_flags(context, 0, 0, 0, cset);
}

static void proc_rla(cpu_context *context) {
    unsigned char temp = context->regs.a;
    unsigned char cset = (temp >> 7) & 1; 

    context->regs.a = (temp << 1) | CPU_CFLAG;
    cpu_flags(context, 0, 0, 0, cset);
}

static void proc_rra(cpu_context *context) {
    unsigned char cflag = CPU_CFLAG;
    unsigned char new_cflag = context->regs.a & 1;
    context->regs.a >>= 1;
    context->regs.a |= (cflag << 7);

    cpu_flags(context, 0, 0, 0, new_cflag);
}

static void proc_stop(cpu_context *context) {
    std::cout << "Stopping..." << std::endl;
    NOT_IMPL
}

static void proc_daa(cpu_context *context) {
    unsigned char temp = 0;
    int cflag = 0;

    if(CPU_HFLAG || (!CPU_NFLAG && (context->regs.a & 0xF) > 9))
        temp = 6;
    
    if(CPU_CFLAG || (!CPU_NFLAG && context->regs.a > 0x99)) {
        temp |= 0x60;
        cflag = 1;
    }

    context->regs.a += CPU_NFLAG ? -(temp) : temp;

    cpu_flags(context, context->regs.a == 0, -1, 0, cflag);
}

static void proc_cpl(cpu_context *context) {
    context->regs.a = ~context->regs.a;
    cpu_flags(context, -1, 1, 1, -1);
}

static void proc_scf(cpu_context *context) {
    cpu_flags(context, -1, 0, 0, 1);
}

static void proc_ccf(cpu_context *context) {
    cpu_flags(context, -1, 0, 0, CPU_CFLAG ^ 1);
}

static void proc_halt(cpu_context *context) {
    context->halt = true;
}