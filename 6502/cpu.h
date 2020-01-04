#pragma once

#include "bus.h"

class CPU {
    public:
    /* Registers */
    uint8_t AC, X, Y, SR, SP; /* Acumulator, x, y, status, stack pointer */
    uint16_t PC; /* Program counter */

    std::unique_ptr<BUS> bus;
    void set_bus(std::unique_ptr<BUS>);

    void reset();

    CPU();
    ~CPU();

    void exec(const uint8_t);

    private:
    enum bs {
        N, O, U, B, D, I, Z, C // Negative, Overflow, Unused, Break, Decimal, Interrupt, Zero, Carry
    };

};

