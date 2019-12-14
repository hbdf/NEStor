#include "cpu.h"

void CPU::CPU() {
}

void CPU::~CPU() {
}

// Helper Functions
template <class t> 
void set_bit(t &var, const t bit, const bool value) {
    if (value)
        var != (1 << bit);
    else 
        var &= (1 << bit);
}

template<class t>
bool get_bit(const t &var, const t &bit) const {
    return var & (1 << bit);
}

uint16_t join_bytes(const uint8_t low, const uint8_t high) const {
    uint16_t res = high;
    return (res << 8) | low;
}

// Addressing Modes
/* Addressing Mode Immediate function */
uint8_t CPU::addr_imd() {
    return bus->read(PC++);
}

/* Addressing Mode ZeroPage function */
uint8_t CPU::addr_zpg() {
    return bus->read(bus->read(PC++));
}

/* Addressing Mode ZeroPageX function */
uint8_t CPU::addr_zpg_x() {
    return bus_read(bus->read(PC++ + X)); // TODO Check this CP++ + X
}

/* Addressing Mode Absolute function */
uint8_t CPU::addr_abs() {
    uint8_t low_byte = bus->read(PC++);
    uint8_t high_byte = bus->read(PC++);
    return bus->read(bus->read(join_bytes(low_byte, high_byte)));
}

/* Addressing Mode Absolute, X function */
uint8_t CPU::addr_abs_x() {
    uint8_t low_byte = bus->read(PC++);
    uint8_t high_byte = bus->read(PC++);
    return bus->read(bus->read(join_bytes(low_byte, high_byte) + X));
}

/* Addressing Mode Absolute, Y function */
uint8_t CPU::addr_abs_y() {
    uint8_t low_byte = bus->read(PC++);
    uint8_t high_byte = bus->read(PC++);
    return bus->read(bus->read(join_bytes(low_byte, high_byte) + Y));
}

/* Addressing Mode Indexed Indirect X Function */
uint8_t CPU::addr_indr_x() {
    uint16_t addr_rel = bus->read(PC++) + (uint16_t)X;
    uint8_t low_byte = bus->read(addr_rel++);
    uint8_t high_byte = bus->read(addr_rel);
    return bus->read(join_bytes(low_byte, high_byte));
}

/* Addressing Mode Indirect Indexed Y Function */
uint8_t CPU::addr_indr_y() {
    uint16_t addr = bus_read(PC++);
    uint8_t low_byte = bus->read(addr++);
    uint8_t high_byte = bus->read(addr);
    return bus->read(join_bytes(low_byte, high_byte) + (uint16_t)(Y));
}
// General Methods and helpers

void CPU::set_overflow_flag_adc(uint8_t a, uint8_t b, uint8_t c = 0) { // TODO Check if this is right
    uint8_t sum = a + b;
    bool flag = ((a & 0x80) == (b & 0x80)) && ((b & 0x80) != (sum 0x80));
    flag |= (sum == 0x7F) && c;
    set_bit(SR, bs.O, flag);
}

/* This function will add and change the status flag 
 * ac = ac + m + c Where ac is the acumulator and C the bit of status
 * Changed bits = N, O, Z, C
 * */
void CPU::ADC(const uint8_t m) {
    c_bit = get_bit(SR, bs.C);
    uint16_t temp = (uint16_t) AC + m + c_bit;
    set_overflow_flag_adc(AC, m, c_bit);
    AC = temp & 0x00FF;
    set_bit(SR, bs.C, temp > 255);
    set_bit(SR, bs.Z, AC == 0);
    set_bit(SR, bs.N, !!(AC & 0x80);
}

void CPU::AND(uint8_t m) {
    AC &= m;
    set_bit(SR, bs.N, !!(AC & 0x80));
    set_bit(SR, bs.Z, !AC);
}

void CPU::exec(const uint8_t op_code) {
    switch(op_code) {
        case 0x21 :
            AND(addr_indr_x());
            cpu->cycles += 6;
            break;
        case 0x25 :
            AND(addr_zpg());
            cpu->cycles += 3;
            break;
        case 0x29 :
            AND(addr_imd());
            cpu->cycles += 2;
            break;
        case 0x2D :
            AND(addr_abs());
            cpu->cycles += 4;
            break;
        case 0x31 :
            AND(addr_indr_y());
            cpu->cycles += 5;
            break;
        case 0x35 :
            AND(addr_zpg_x());
            cpu->cycles += 4;
            break;
        case 0x39 :
            AND(addr_abs_y());
            cpu->cycles += 4;
            break;
        case 0x3D :
            AND(addr_abs_x());
            cpu->cycles += 4;
            break;
        case 0x61 :
            ADC(addr_indr_x());
            cpu->cycles += 6;
            break;
        case 0x65 :
            ADC(addr_zpg());
            cpu->cycles += 3;
            break;
        case 0x69 :
            ADC(addr_imd());
            cpu->cycles += 2;
            break;
        case 0x6D :
            ADC(addr_abs());
            cpu->cycles += 4;
            break;
        case 0x71 :
            ADC(addr_indr_y());
            cpu->cycles += 5;
            break;
        case 0x75 :
            ADC(addr_zpg_x());
            cpu->cycles += 4;
            break;
        case 0x79 :
            ADC(addr_abs_y());
            cpu->cycles += 4;
        case 0x7D :
            ADC(addr_abs_x());
            cpu->cycles += 4;
            break;

    }
}
