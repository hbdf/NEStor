#include "cpu.h"

void CPU::CPU() {
}

void CPU::~CPU() {
}

// Helper Functions
template <class t> 
void set_bit(t &var, const t bit, const bool value) {
    var |= (1 << bit);
    if (!value)
        var ^= (1 << bit);
}

template<class t>
bool get_bit(const t var, const t bit) const {
    return !!(var & (1 << bit));
}

uint16_t join_bytes(const uint8_t low, const uint8_t high) const {
    uint16_t res = high;
    return (res << 8) | low;
}

// Addressing Modes
/* Addressing Mode Immediate function */
uint16_t CPU::addr_imd() {
    return (PC++);
}

/* Addressing Mode ZeroPage function */
uint16_t CPU::addr_zpg() {
    return (bus->read(PC++));
}

/* Addressing Mode ZeroPageX function */
uint16_t CPU::addr_zpg_x() {
    return (bus->read(PC++ + X)); // TODO Check this CP++ + X
}

/* Addressing Mode Absolute function */
uint16_t CPU::addr_abs() {
    uint8_t low_byte = bus->read(PC++);
    uint8_t high_byte = bus->read(PC++);
    return (bus->read(join_bytes(low_byte, high_byte)));
}

/* Addressing Mode Absolute, X function */
uint16_t CPU::addr_abs_x() {
    uint8_t low_byte = bus->read(PC++);
    uint8_t high_byte = bus->read(PC++);
    return (bus->read(join_bytes(low_byte, high_byte) + X));
}

/* Addressing Mode Absolute, Y function */
uint16_t CPU::addr_abs_y() {
    uint8_t low_byte = bus->read(PC++);
    uint8_t high_byte = bus->read(PC++);
    return (bus->read(join_bytes(low_byte, high_byte) + Y));
}

/* Addressing Mode Indirect */
uint16_t CPU::addr_indr() {
    uint16_t addr = addr_abs();
    uint8_t low_byte = bus->read(addr);
    uint8_t high_byte = bus->read(addr + 1);
    return join_bytes(low_byte, high_byte);
}

/* Addressing Mode Indexed Indirect X Function */
uint16_t CPU::addr_indr_x() {
    uint16_t addr_rel = bus->read(PC++) + (uint16_t)X;
    uint8_t low_byte = bus->read(addr_rel++);
    uint8_t high_byte = bus->read(addr_rel);
    return (join_bytes(low_byte, high_byte));
}

/* Addressing Mode Indirect Indexed Y Function */
uint16_t CPU::addr_indr_y() {
    uint16_t addr = bus_read(PC++);
    uint8_t low_byte = bus->read(addr++);
    uint8_t high_byte = bus->read(addr);
    return (join_bytes(low_byte, high_byte) + (uint16_t)(Y));
}

void CPU::addr_rel(bool branch) {
    uint8_t pc_content = read(PC++);
    if(branch)
        PC += pc_content;
}

uint8_t CPU::read(std::function<uint16_t(void)> addr_mode) {
    return bus->read(addr_mode());
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

uint8_t CPU::ASL(uint8_t val) {
    set_bit(SR, bs.C, !!(val & 0x80));
    val <<= 1;
    set_bit(SR, bs.N, !!(val & 0x80));
    set_bit(SR, bs.Z, val == 0);
    return val;
}

void CPU::BCC() {
    addr_rel(!get_bit(SR, bs.C));
}

void CPU::BCS() {
    addr_rel(get_bit(SR, bs.C));
}

void CPU::BEQ() {
    addr_rel(get_bit(SR, bs.Z));
}

void CPU::BIT(const uint8_t m) {
    AC &= m;
    set_bit(SR, bs.Z, AC == 0);
    set_bit(SR, bs.N, get_bit(m, 7));
    set_bit(SR, bs.O, get_bit(m, 6));
}

void CPU::BMI() {
    addr_rel(get_bit(SR, bs.N));
}

void CPU::BNE() {
    addr_rel(!get_bit(SR, bs.Z));
}

void CPU::BPL() {
    addr_rel(!get_bit(SR, bs.N));
}

void CPU::BRK() {
    uint16_t pc = PC;
    bus->write(SP--, pc >> 8);
    bus->write(SP--, pc & 0x00FF);
    bus->write(SP--, SR);
    PC = join_bytes(bus->read(0xFFFE), bus->read(0xFFFF));
    SR = (1 << bs.I);
}

void CPU::BVC() {
    addr_rel(!get_bit(SR, bs.V));
}

void CPU::BVS() {
    addr_rel(get_bit(SR, bs.V));
}

void CPU::CLC() {
    set_bit(SR, bs.C, 0);
}

void CPU::CLD() {
    set_bit(SR, bs.D, 0);
}

void CPU::CLI() {
    set_bit(SR, bs.I, 0);
}

void CPU::CLV() {
    set_bit(SR, bs.O, 0);
}

void CPU::CMP(uint8_t m) {
    uint8_t comp = AC - m;
    set_bit(SR, bs.N, !!(comp & 0x80));
    set_bit(SR, bs.Z, !comp);
    set_bit(SR, bs.C, ((uint16_t) AC - (uint16_t) m) != comp);
}

void CPU::CPX(uint8_t m) {
    uint8_t comp = X - m;
    set_bit(SR, bs.N, !!(comp & 0x80));
    set_bit(SR, bs.Z, !comp);
    set_bit(SR, bs.C, ((uint16_t) X - (uint16_t) m) != comp);
}

void CPU::CPY(uint8_t m) {
    uint8_t comp = Y - m;
    set_bit(SR, bs.N, !!(comp & 0x80));
    set_bit(SR, bs.Z, !comp);
    set_bit(SR, bs.C, ((uint16_t) Y - (uint16_t) m) != comp);
}

void CPU::DEC(uint16_t memory_location) {
    uint8_t val = read(memory_location) - 1;
    set_bit(SR, bs.N, !!(val & 0x80));
    set_bit(SR, bs.Z, !val);
    bus->write(memory_location, val);
}

void CPU::DEX() {
    X--;
    set_bit(SR, bs.N, !!(X & 0x80));
    set_bit(SR, bs.Z, !X);
}

void CPU::DEY() {
    Y--;
    set_bit(SR, bs.N, !!(Y & 0x80));
    set_bit(SR, bs.Z, !Y);
}

void CPU::EOR(uint8_t m) {
    AC ^= m;
    set_bit(SR, bs.N, !!(AC & 0x80));
    set_bit(SR, bs.Z, !AC);
}

void CPU::INC(uint16_t memory_location) {
    uint8_t val = read(memory_location);
    val++;
    set_bit(SR, bs.N, !!(val &0x80));
    set_bit(SR, bs.Z, !val);
    bus->write(memory_location, val);
}

void CPU::INX() {
    X++;
}

void CPU::INY() {
    Y++;
}

void CPU::JMP(uint16_t memory_location) {
    PC = memory_location;
}

void CPU::JSR(uint16_t memory_location) {
    bus->write(SP--, PC >> 8);
    bus->write(SP--, PC & 0x00FF);
    PC = memory_location;
}

void CPU::LDA(uint8_t m) {
    AC = m;
    set_bit(SR, bs.N, !!(AC &0x80));
    set_bit(SR, bs.Z, !AC);
}

void CPU::LDX(uint8_t m) {
    X = m;
    set_bit(SR, bs.N, !!(X &0x80));
    set_bit(SR, bs.Z, !X);
}

void CPU::LDY(uint8_t m) {
    Y = m;
    set_bit(SR, bs.N, !!(Y &0x80));
    set_bit(SR, bs.Z, !Y);
}

/* Only works for memory, not with acumulator */
void CPU::LSR(uint16_t memory_location) {
    uint8_t val = bus->read(memory_location);
    set_bit(SR, bs.C, !!(val & 0x1));
    val >>= 1;
    set_bit(SR, bs.Z, !val);
    set_bit(SR, bs.N, false);
    bus->write(memory_location, val);
}

void CPU::NOP() {}

void CPU::ORA(uint8_t m) {
    AC |= m;
    set_bit(SR, bs.Z, !AC);
    set_bit(SR, bs.N, !!(AC & 0x80));
}

void CPU::PHA() {
    bus->write(SP--, AC);
}

void CPU:PHP() {
    bus->write(SP--, SR);
}

void CPU::PLA() {
    uint8_t val = bus->read(SP++);
    set_bit(SR, bs.N, !!(val & 0x80));
    set_bit(SR, bs.Z, !val);
    AC = val;
}

void CPU::PLP() {
    uint8_t val = bus->read(SP++);
    set_bit(SR, bs.N, !!(val & 0x80));
    set_bit(SR, bs.Z, !val);
    SR = val;
}

void CPU::exec(const uint8_t op_code) {
    switch(op_code) {
        case 0x00 :
            BRK();
            this->cycles += 7;
            break;
        case 0x01 :
            ORA(read(addr_indr_x));
            this->cycles += 6;
            break;
        case 0x05 :
            ORA(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0x06 :
            uint16_t mem_location = addr_zpg();
            uint8_t val = bus->read(mem_location);
            bus->write(mem_location, ASL(val));
            this->cycles += 5;
            break;
        case 0x08 :
            PHP();
            this->cycles += 3;
            break;
        case 0x09 :
            ORA(read(addr_imd));
            this->cycles += 2;
            break;
        case 0x0A :
            uint8_t val = ASL(AC);
            AC = val;
            this->cycles += 2;
            break;
        case 0x0D :
            ORA(read(addr_abs));
            this->cycles += 4;
            break;
        case 0x0E :
            uint16_t mem_location = addr_abs();
            uint8_t val = bus->read(mem_location);
            bus->write(mem_location, ASL(val));
            this->cycles += 6;
            break;
        case 0x10 :
            BPL();
            this->cycles += 2;
            break;
        case 0x11 :
            ORA(read(addr_indr_y));
            this->cycles += 5;
            break;
        case 0x15 :
            ORA(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0x16 :
            uint16_t mem_location = addr_zpg_x();
            uint8_t val = bus->read(mem_location);
            bus->write(mem_location, ASL(val));
            this->cycles += 6;
            break;
        case 0x18 :
            CLC();
            this->cycles += 2;
            break;
        case 0x19 :
            ORA(read(addr_abs_y));
            this->cycles += 4;
            break;
        case 0x1D :
            ORA(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0x1E :
            uint16_t mem_location = addr_abs_x();
            uint8_t val = bus->read(mem_location);
            bus->write(mem_location, ASL(val));
            this->cycles += 7;
            break;
        case 0x20 :
            JSR(addr_abs());
            this->cycles += 6;
            break;
        case 0x21 :
            AND(read(addr_indr_x));
            this->cycles += 6;
            break;
        case 0x24 :
            BIT(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0x25 :
            AND(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0x28 :
            PLP();
            this->cycles += 4;
            break;
        case 0x29 :
            AND(read(addr_imd));
            this->cycles += 2;
            break;
        case 0x2C :
            BIT(read(addr_abs));
            this->cycles += 4;
            break;
        case 0x2D :
            AND(read(addr_abs));
            this->cycles += 4;
            break;
        case 0x30 :
            BMI();
            this->cycles += 2;
            break;
        case 0x31 :
            AND(read(addr_indr_y));
            this->cycles += 5;
            break;
        case 0x35 :
            AND(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0x39 :
            AND(read(addr_abs_y));
            this->cycles += 4;
            break;
        case 0x3D :
            AND(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0x41 :
            EOR(read(addr_indr_x));
            this->cycles += 6;
            break;
        case 0x45 :
            EOR(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0x46 :
            LSR(addr_zpg());
            this->cycles += 5;
            break;
        case 0x48 :
            PHA();
            this->cycles += 3;
            break;
        case 0x49 :
            EOR(read(addr_imd));
            this->cycles += 2;
            break;
        case 0x4A :
            set_bit(SR, bs.C, !!(AC & 0x1));
            AC >>= 1;
            set_bit(SR, bs.Z, !AC);
            set_bit(SR, bs.N, false);
            this->cycles += 2;
            break;
        case 0x4C :
            JMP(addr_abs());
            this->cycles += 3;
            break;
        case 0x4D :
            EOR(read(addr_abs));
            this->cycles += 4;
            break;
        case 0x4E :
            LSR(addr_abs());
            this->cycles += 6;
            break;
        case 0x50 :
            BVC();
            this->cycles += 2;
            break;
        case 0x51 :
            EOR(read(addr_indr_y));
            this->cycles += 5;
            break;
        case 0x55 :
            EOR(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0x56 :
            LSR(addr_zpg_x());
            this->cycles += 6;
            break;
        case 0x58 :
            CLI();
            this->cycles += 2;
            break;
        case 0x59 :
            EOR(read(addr_abs_y));
            this->cycles += 4;
            break;
        case 0x5D :
            EOR(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0x5E :
            LSR(addr_abs_x());
            this->cycles += 7;
            break;
        case 0x61 :
            ADC(read(addr_indr_x));
            this->cycles += 6;
            break;
        case 0x65 :
            ADC(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0x68 :
            PLA();
            this->cycles += 4;
            break;
        case 0x69 :
            ADC(read(addr_imd));
            this->cycles += 2;
            break;
        case 0x6C :
            JMP(addr_indr());
            this->cycles += 5;
            break;
        case 0x6D :
            ADC(read(addr_abs));
            this->cycles += 4;
            break;
        case 0x70 :
            BVS();
            this->cycles += 2;
            break;
        case 0x71 :
            ADC(read(addr_indr_y));
            this->cycles += 5;
            break;
        case 0x75 :
            ADC(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0x79 :
            ADC(read(addr_abs_y));
            this->cycles += 4;
            break;
        case 0x7D :
            ADC(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0x88 :
            DEY();
            this->cycles += 2;
            break;
        case 0x90 :
            BCC();
            this->cycles += 2;
            break;
        case 0xA0 :
            LDY(read(add_imd));
            this->cycles += 2;
            break;
        case 0xA1 :
            LDA(read(addr_indr_x));
            this->cycles += 6;
            break;
        case 0xA2 :
            LDX(read(addr_imd));
            this->cycles += 2;
            break;
        case 0xA4 :
            LDY(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0xA5 :
            LDA(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0xA6 :
            LDX(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0xA9 :
            LDA(read(addr_imd));
            this->cycles += 2;
            break;
        case 0xAC :
            LDY(read(addr_abs));
            this->cycles += 4;
            break;
        case 0xAD :
            LDA(read(addr_abs));
            this->cycles += 4;
            break;
        case 0xAE :
            LDX(read(addr_abs));
            this->cycles += 4;
            break;
        case 0xB0 :
            BCS();
            this->cycles += 2;
            break;
        case 0xB1 :
            LDA(read(addr_indr_y));
            this->cycles += 5;
            break;
        case 0xB4 :
            LDY(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0xB5 :
            LDA(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0xB6 :
            LDX(read(addr_zpg_y));
            this->cycles += 4;
            break;
        case 0xB8 :
            CLV();
            this->cycles += 2;
            break;
        case 0xB9 :
            LDA(read(addr_abs_y));
            this-cycles += 4;
            break;
        case 0xBC :
            LDY(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0xBD :
            LDA(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0xBE :
            LDX(read(addr_abs_y));
            this->cycles += 4;
            break;
        case 0xC0 :
            CPY(read(addr_imd));
            this->cycles += 2;
            break;
        case 0xC1 :
            CMP(read(addr_indr_x));
            this->cycles += 6;
            break;
        case 0xC4 :
            CPY(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0xC5 :
            CMP(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0xC6 :
            DEC(addr_zpg());
            this->cycles += 5;
            break;
        case 0xC8 :
            INY();
            this->cycles += 2;
            break;
        case 0xC9 :
            CMP(read(addr_imd));
            this->cycles += 2;
            break;
        case 0xCA :
            DEX();
            this->cycles += 2;
            break;
        case 0xCC :
            CPY(read(addr_abs));
            this->cycles += 4;
            break;
        case 0xCD :
            CMP(read(addr_abs));
            this->cycles += 4;
            break;
        case 0xCE :
            DEC(addr_abs());
            this->cycles += 6;
            break;
        case 0xD0 :
            BNE();
            this->cycles += 2;
            break;
        case 0xD1 :
            CMP(read(addr_indr_y));
            this->cycles += 5;
            break;
        case 0xD5 :
            CMP(read(addr_zpg_x));
            this->cycles += 4;
            break;
        case 0xD6 :
            DEC(addr_zpg_x());
            this->cycles += 6;
            break;
        case 0xD8 :
            CLD();
            this->cycles += 2;
            break;
        case 0xD9 :
            CMP(read(addr_abs_y));
            this->cycles += 4;
            break;
        case 0xDD :
            CMP(read(addr_abs_x));
            this->cycles += 4;
            break;
        case 0xDE :
            DEC(addr_abs_x());
            this->cycles += 7;
            break;
        case 0xE0 :
            CPX(read(addr_imd));
            this->cycles += 2;
            break;
        case 0xE4 :
            CPX(read(addr_zpg));
            this->cycles += 3;
            break;
        case 0xE6 :
            INC(addr_zpg());
            this->cycles += 5;
            break;
        case 0xE8 :
            INX();
            this->cycles += 2;
            break;
        case 0xEA :
            NOP();
            this->cycles += 2;
            break;
        case 0xEC :
            CPX(read(addr_abs));
            this->cycles += 4;
            break;
        case 0xEE :
            INC(addr_abs());
            this->cycles += 6;
            break;
        case 0xF6 :
            INC(addr_zpg_x());
            this->cycles += 6;
            break;
        case 0xFE :
            INC(addr_abs_x());
            this->cycles += 7;
            break;

    }
}
