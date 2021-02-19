#pragma once
#include <stdint.h>

class Chip8{
private:
    uint8_t memory[4096];
    uint8_t V[16];

    uint16_t stack[16];
    uint16_t sp;  //points to topmost level of stack

    uint16_t pc; //stores currently executing address 
    uint16_t opcode;
    uint16_t I;  //Index register 

    uint8_t soundTimer;
    uint8_t delayTimer;

    void init();
public:
    uint8_t graphics[64 * 32];
    uint8_t keys[16];

    bool drawFlag;

    Chip8();
    ~Chip8();

    void emulateCycle();
    bool load(const char* file_path);
};