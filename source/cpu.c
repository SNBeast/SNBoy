#include "cpu.h"
#include "memory.h"

struct CPU_State cpu;

void add8 (u8 value) {
    u16 old_a = cpu.a;
    cpu.a += value;

    cpu.zero = !cpu.a;
    cpu.subtract = 0;
    cpu.half_carry = ((old_a & 0x0f) + (value & 0x0f)) > 0x0f;
    cpu.carry = (old_a + value) > 0xff;
}
void adc (u8 value) {
    u16 old_a = cpu.a;
    cpu.a += value + cpu.carry;

    cpu.zero = !cpu.a;
    cpu.subtract = 0;
    cpu.half_carry = ((old_a & 0x0f) + (value & 0x0f) + cpu.carry) > 0x0f;
    cpu.carry = (old_a + value + cpu.carry) > 0xff;
}
void sub (u8 value) {
    s16 old_a = cpu.a;
    cpu.a -= value;

    cpu.zero = !cpu.a;
    cpu.subtract = 1;
    cpu.half_carry = ((old_a & 0x0f) - (value & 0x0f)) < 0;
    cpu.carry = (old_a - value) < 0;
}
void sbc (u8 value) {
    s16 old_a = cpu.a;
    cpu.a -= value + cpu.carry;

    cpu.zero = !cpu.a;
    cpu.subtract = 1;
    cpu.half_carry = ((old_a & 0x0f) - (value & 0x0f) - cpu.carry) < 0;
    cpu.carry = (old_a - value - cpu.carry) < 0;
}
void and (u8 value) {
    cpu.a &= value;

    cpu.f = cpu.a == 0 ? 0b1010'0000 : 0b0010'0000;
}
void xor (u8 value) {
    cpu.a ^= value;

    cpu.f = cpu.a == 0 ? 0b1000'0000 : 0b0000'0000;
}
void or (u8 value) {
    cpu.a |= value;

    cpu.f = cpu.a == 0 ? 0b1000'0000 : 0b0000'0000;
}
void cp (u8 value) {
    cpu.zero = cpu.a - value == 0;
    cpu.subtract = 1;
    cpu.half_carry = ((cpu.a & 0x0f) - (value & 0x0f) - cpu.carry) < 0;
    cpu.carry = (cpu.a - value - cpu.carry) < 0;
}

void rlc (u8* reg) {
    cpu.f = (!*reg << 7) | (!!(*reg & 0x80) << 4);
    *reg = (*reg << 1) | cpu.carry;
}
void rrc (u8* reg) {
    cpu.f = (!*reg << 7) | ((*reg & 0x01) << 4);
    *reg = (*reg >> 1) | (cpu.carry << 7);
}
void rl (u8* reg) {
    bool old_carry = cpu.carry;
    cpu.f = (!!(*reg & 0x80) << 4);
    *reg = (*reg << 1) | old_carry;
    
    cpu.zero = !*reg;
}
void rr (u8* reg) {
    bool old_carry = cpu.carry;
    cpu.f = ((*reg & 0x01) << 4);
    *reg = (*reg >> 1) | (old_carry << 7);
    
    cpu.zero = !*reg;
}
void sla (u8* reg) {
    cpu.f = (!*reg << 7) | (!!(*reg & 0x80) << 4);
    *reg <<= 1;
}
void sra (u8* reg) {
    cpu.f = (!*reg << 7) | ((*reg & 0x01) << 4);
    *reg = (*reg & 0x80) | (*reg >> 1);
}
void swap (u8* reg) {
    *reg = (*reg << 4) | (*reg >> 4);
    
    cpu.f = *reg == 0 ? 0b1000'0000 : 0b0000'0000;
}
void srl (u8* reg) {
    cpu.f = (!*reg << 7) | ((*reg & 0x01) << 4);
    *reg >>= 1;
}

void bit (u8 which, u8 value) {
    cpu.zero = !(value & (1 << which));
    cpu.subtract = 0;
    cpu.half_carry = 1;
}

void res (u8 which, u8* reg) {
    *reg &= ~(1 << which);
}
void set (u8 which, u8* reg) {
    *reg |= 1 << which;
}

void push (u16 value) {
    write16(cpu.sp - 1, value);
    cpu.sp -= 2;
}
u16 pop (void) {
    u16 retVal = read16(cpu.sp);
    cpu.sp += 2;
    return retVal;
}
void jr (s8 offset, bool condition) {
    if (condition) {
        cpu.pc += offset;
    }
}
void jp (u16 place, bool condition) {
    if (condition) {
        cpu.pc = place;
    }
}
void call (u16 place, bool condition) {
    cpu.pc += 2;
    if (condition) {
        push(cpu.pc);
        cpu.pc = place;
    }
}
void ret (bool condition, bool i) {
    if (condition) {
        cpu.pc = pop();
        if (i) {
            cpu.ime = 1;
        }
    }
}

void add16 (u16 value) {
    u32 old_hl = cpu.hl;
    cpu.hl += value;

    cpu.zero = 0;
    cpu.subtract = 0;
    cpu.half_carry = ((old_hl & 0x0fff) + (value & 0x0fff)) > 0x0fff;
    cpu.carry = (old_hl + value) > 0xffff;
}
void addSP (s8 offset) {
    u16 old_sp = cpu.sp;
    cpu.sp += offset;

    cpu.zero = 0;
    cpu.subtract = 0;
    cpu.half_carry = ((old_sp & 0x000f) + (offset & 0x000f)) > 0x0f;
    cpu.carry = ((old_sp & 0x00ff) + (offset & 0xff)) > 0xff;
}
void ldHLSP (s8 offset) {
    cpu.hl = cpu.sp + offset;

    cpu.zero = 0;
    cpu.subtract = 0;
    cpu.half_carry = ((cpu.sp & 0x000f) + (offset & 0x000f)) > 0x0f;
    cpu.carry = ((cpu.sp & 0x00ff) + (offset & 0xff)) > 0xff;
}

void inc (u8* reg) {
    (*reg)++;

    cpu.zero = !*reg;
    cpu.subtract = 0;
    cpu.half_carry = !(*reg & 0x0f);
}
void dec (u8* reg) {
    (*reg)--;

    cpu.zero = !*reg;
    cpu.subtract = 1;
    cpu.half_carry = (*reg & 0x0f) == 0x0f;
}

void rlca(void) {
    cpu.f = (!!(cpu.a & 0x80) << 4);
    cpu.a = (cpu.a << 1) | cpu.carry;
}
void rrca(void) {
    cpu.f = ((cpu.a & 0x01) << 4);
    cpu.a = (cpu.a >> 1) | (cpu.carry << 7);
}
void rla(void) {
    bool old_carry = cpu.carry;
    cpu.f = (!!(cpu.a & 0x80) << 4);
    cpu.a = (cpu.a << 1) | old_carry;
}
void rra(void) {
    bool old_carry = cpu.carry;
    cpu.f = ((cpu.a & 0x01) << 4);
    cpu.a = (cpu.a >> 1) | (old_carry << 7);
}
void daa(void) {
    u16 value = cpu.a;
    if (cpu.subtract) {
        if (cpu.half_carry) {
            value -= 0x06;
        }
        if (cpu.carry) {
            value -= 0x60;
        }
    }
    else {
        if (cpu.half_carry || (value & 0x0f) > 0x09) {
            value += 0x06;
        }
        if (cpu.carry || value > 0x9f) {
            value += 0x60;
        }
    }
    cpu.a = value & 0xff;

    cpu.zero = !cpu.a;
    cpu.subtract = 1;
    cpu.carry = value & 0x100;
}
void cpl(void) {
    cpu.a ^= 0xff;

    cpu.subtract = 1;
    cpu.half_carry = 1;
}

// used for instruction decoder
// reference: https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
typedef bool (*Condition) (void);
typedef void (*Function8) (u8);
typedef void (*FunctionReg) (u8*);
typedef void (*FunctionHL) (void);
bool notCarry (void) {
    return !cpu.carry;
}
bool carry (void) {
    return cpu.carry;
}
bool notZero (void) {
    return !cpu.zero;
}
bool zero (void) {
    return cpu.zero;
}
u8* r[] = {&cpu.b, &cpu.c, &cpu.d, &cpu.e, &cpu.h, &cpu.l, NULL, &cpu.a}; // [hl] is left as NULL because that needs to be handled differently, so we should crash if unhandled
u16* rp[] = {&cpu.bc, &cpu.de, &cpu.hl, &cpu.sp};
u16* rp2[] = {&cpu.bc, &cpu.de, &cpu.hl, &cpu.af};
Condition cc[] = {notCarry, carry, notZero, zero};
Function8 alu[] = {add8, adc, sub, sbc, and, xor, or, cp};
FunctionReg rot[] = {rlc, rrc, rl, rr, sla, sra, swap, srl};

void step (void) {
    u8 instruction = read8(cpu.pc++);

    if (instruction == 0xCB) {
        instruction = read8(cpu.pc++);
        u8 x = instruction >> 6;
        u8 y = (instruction >> 3) & 0b111;
        u8 z = instruction & 0b111;
        switch (x) {
            case 0:
                if (z == 6) {
                    u8 hlDeref = read8(cpu.hl);
                    rot[y](&hlDeref);
                    write8(cpu.hl, hlDeref);
                }
                else {
                    rot[y](r[z]);
                }
                break;
            case 1:
                if (z == 6) {
                    bit(y, read8(cpu.hl));
                }
                else {
                    bit(y, *r[z]);
                }
                break;
            case 2:
                if (z == 6) {
                    u8 hlDeref = read8(cpu.hl);
                    res(y, &hlDeref);
                    write8(cpu.hl, hlDeref);
                }
                else {
                    res(y, r[z]);
                }
                break;
            case 3:
                if (z == 6) {
                    u8 hlDeref = read8(cpu.hl);
                    set(y, &hlDeref);
                    write8(cpu.hl, hlDeref);
                }
                else {
                    set(y, r[z]);
                }
                break;
        }
    }
    else {
        u8 x = instruction >> 6;
        u8 y = (instruction >> 3) & 0b111;
        u8 z = instruction & 0b111;
        u8 p = y >> 1;
        u8 q = y & 1;
        switch (x) {
            case 0:
                switch (z) {
                    case 0:
                        switch (y) {
                            case 0:
                                // nop
                                break;
                            case 1:
                                write16(read16(cpu.pc), cpu.sp);
                                cpu.pc += 2;
                                break;
                            case 2:
                                // TODO: stop
                                break;
                            case 3:
                                jr((s8)read8(cpu.pc++), 1);
                                break;
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                                jr((s8)read8(cpu.pc++), cc[y - 4]());
                                break;
                        }
                        break;
                    case 1:
                        if (q) {
                            add16(*rp[p]);
                        }
                        else {
                            *rp[p] = read16(cpu.pc);
                            cpu.pc += 2;
                        }
                        break;
                    case 2:
                        if (q) {
                            switch (p) {    
                                case 0:
                                    cpu.a = read8(cpu.bc);
                                    break;
                                case 1:
                                    cpu.a = read8(cpu.de);
                                    break;
                                case 2:
                                    cpu.a = read8(cpu.hl++);
                                    break;
                                case 3:
                                    cpu.a = read8(cpu.hl--);
                                    break;
                            }
                        }
                        else {
                            switch (p) {
                                case 0:
                                    write8(cpu.bc, cpu.a);
                                    break;
                                case 1:
                                    write8(cpu.de, cpu.a);
                                    break;
                                case 2:
                                    write8(cpu.hl++, cpu.a);
                                    break;
                                case 3:
                                    write8(cpu.hl--, cpu.a);
                                    break;
                            }
                        }
                        break;
                    case 3:
                        // 16-bit inc and dec use the logic for sp and pc, so they don't touch flags
                        if (q) {
                            (*rp[p])--;
                        }
                        else {
                            (*rp[p])++;
                        }
                        break;
                    case 4:
                        if (y == 6) {
                            u8 hlDeref = read8(cpu.hl);
                            inc(&hlDeref);
                            write8(cpu.hl, hlDeref);
                        }
                        else {
                            inc(r[y]);
                        }
                        break;
                    case 5:
                        if (y == 6) {
                            u8 hlDeref = read8(cpu.hl);
                            dec(&hlDeref);
                            write8(cpu.hl, hlDeref);
                        }
                        else {
                            dec(r[y]);
                        }
                        break;
                    case 6:
                        if (y == 6) {
                            write8(cpu.hl, read8(cpu.pc++));
                        }
                        else {
                            *r[y] = read8(cpu.pc++);
                        }
                        break;
                    case 7:
                        switch (y) {
                            case 0:
                                rlca();
                                break;
                            case 1:
                                rrca();
                                break;
                            case 2:
                                rla();
                                break;
                            case 3:
                                rra();
                                break;
                            case 4:
                                daa();
                                break;
                            case 5:
                                cpl();
                                break;
                            case 6:
                                cpu.carry = 1;
                                break;
                            case 7:
                                cpu.carry ^= 1;
                                break;
                        }
                        break;
                }
                break;
            case 1:
                if (instruction == 0x76) {
                    // TODO: halt
                }
                else {
                    if (y == 6) {
                        write8(cpu.hl, *r[z]);
                    }
                    else if (z == 6) {
                        *r[y] = read8(cpu.hl);
                    }
                    else {
                        *r[y] = *r[z];
                    }
                }
                break;
            case 2:
                if (z == 6) {
                    alu[y](read8(cpu.hl));
                }
                else {
                    alu[y](*r[z]);
                }
                break;
            case 3:
                switch (z) {
                    case 0:
                        switch (y) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                ret(cc[y](), 0);
                                break;
                            case 4:
                                write8(0xff00 | read8(cpu.pc++), cpu.a);
                                break;
                            case 5:
                                addSP(read8(cpu.pc++));
                                break;
                            case 6:
                                cpu.a = read8(0xff00 | read8(cpu.pc++));
                                break;
                            case 7:
                                ldHLSP(read8(cpu.pc++));
                                break;
                        }
                        break;
                    case 1:
                        if (!q) {
                            *rp2[p] = pop();
                            if (p == 3) {
                                cpu.unused_flags = 0;
                            }
                        }
                        else {
                            switch (p) {
                                case 0:
                                    ret(1, 0);
                                    break;
                                case 1:
                                    ret(1, 1);
                                    break;
                                case 2:
                                    cpu.pc = cpu.hl;
                                    break;
                                case 3:
                                    cpu.sp = cpu.hl;
                                    break;
                            }
                        }
                        break;
                    case 2:
                        if (!q) {
                            jp(read16(cpu.pc), cc[y]());
                        }
                        else {
                            switch (y) {
                                case 4:
                                    write8(0xff00 | cpu.c, cpu.a);
                                    break;
                                case 5:
                                    write8(read16(cpu.pc), cpu.a);
                                    cpu.pc += 2;
                                    break;
                                case 6:
                                    cpu.a = read8(0xff00 | cpu.c);
                                    break;
                                case 7:
                                    cpu.a = read8(read16(cpu.pc));
                                    cpu.pc += 2;
                                    break;
                            }
                        }
                        break;
                    case 3:
                        switch (y) {
                            case 0:
                                jp(read16(cpu.pc), 1);
                                break;
                            case 6:
                                cpu.ime = 0;
                                break;
                            case 7:
                                // TODO: ei, which has a one instruction delay
                                break;
                            // 0xCB, already taken care of
                            case 1:
                            case 2:
                            case 3:
                            case 4:
                            case 5:
                                // TODO: panic
                                break;
                        }
                        break;
                    case 4:
                        switch (y) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                call(read16(cpu.pc), cc[y]());
                                break;
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                                // TODO: panic
                                break;
                        }
                        break;
                    case 5:
                        if (!q) {
                            push(*rp2[p]);
                        }
                        else if (!p) {
                            call(read16(cpu.pc), 1);
                        }
                        else {
                            // TODO: panic
                        }
                        break;
                    case 6:
                        alu[y](read8(cpu.pc++));
                        break;
                    case 7:
                        call(y * 8, 1);
                        break;
                }
                break;
        }
    }
}
