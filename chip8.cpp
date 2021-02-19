#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include "time.h"
#include "DEFINITIONS.h"

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


Chip8::Chip8() {}
Chip8::~Chip8() {}


void Chip8::init(){
    pc = 0x200;
    sp = 0;
    opcode = 0;
    I = 0;

//clear display
    for(int i = 0; i < 2048; i++){
        graphics[i] = 0;
    }

//clear stack keys and V
    for(int i = 0; i < 16; i++){
        stack[i] = 0;
        V[i] = 0;
        keys[i] = 0;
    }

//clear mem
    for(int i = 0; i < 4096; i++){
        memory[i] = 0;
    }

//load fontset into mem
    for(int i = 0; i < 80; i++){
        memory[i] = chip8_fontset[i];
    }

    soundTimer = 0;
    delayTimer = 0;


    // Seed rng
    srand (time(NULL));
}

bool Chip8::load(const char* file_path){
    init();

    FILE *rom = fopen(file_path, "rb");
    if(rom == NULL){
        std::cout << "Failed to open ROM" << std::endl;
        return false;
    }

    //get rom size
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    //allocate in memory
    char *rom_buffer = (char*) malloc(sizeof(char) * rom_size);
    if(rom_buffer == NULL){
        std::cout << "Failed to allocate memory for rom" << std::endl;
        return false;
    }

    //copy rom into buffer
    size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
    if(result != rom_size){
        std::cout << "Failed to read rom" << std::endl;
        return false;
    }
    //copy buffer to mem
    if((4096 - 512) > rom_size){
        for(int i = 0; i < rom_size; i++){
            memory[i + 512] = (uint8_t)rom_buffer[i];
        }
    }
    else{
        std::cout << "ROM too large to fit in memory" << std::endl;
        return false;
    } 

    fclose(rom);
    free(rom_buffer);   

    return true;
}

void Chip8::emulateCycle(){
    //Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1]; //opcode is of two bytes 
    std::cout << std::hex << opcode << " "; 
    std::cout << std::hex << pc << std::endl;
    
    //decode and execute opcode
    switch(opcode & 0xF000){

        case 0x0000:
            switch(opcode & 0x00FF){
                case 0x00E0:
                    //clear screen (00E0)
                    for(int i = 0; i < 2048; i++){
                        graphics[i] = 0;
                    }
                    /*std::cout << std::hex << opcode << " "; 
                    std::cout << std::hex << pc << std::endl;*/
                    drawFlag = true;
                    pc += 2;
                    break;

                case 0x00EE:
                    //return from a subroutine (00EE)
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;
        case 0x1000:
            //Jump to address NNN
            pc = NNN;
            break;
        case 0x2000:
            //call subroutine at NNN
            stack[sp] = pc;
            sp++;
            pc = NNN;
            break;
        case 0x3000:
            //Skip next instruction if VX is equal to NN
            if(VX == NN){
                pc += 4;
            }else{
                pc += 2;
            }
            break;
        case 0x4000:
            if(VX != NN){
                pc += 4;
            }else{
                pc += 2;
            }
            break;
        case 0x5000:
            if(VX == VY){
                pc += 4;
            }else{
                pc += 2;
            }
            break;
        case 0x6000:
            VX = NN;
            pc += 2;
            break;
        case 0x7000:
            VX += NN;
            pc += 2;
            break;
        case 0x8000:
            switch(opcode & 0x000F){
                case 0x0000:
                    VX = VY;
                    pc += 2;
                    break;
                case 0x0001:
                    VX |= VY;
                    pc += 2;  
                    break;
                case 0x0002:
                    VX &= VY;
                    pc += 2;
                    break;
                case 0x0003:
                     VX ^= VY;
                     pc += 2;
                    break;  
                case 0x0004:
                    VX += VY;
                    if(VY + VY > (0xFF - VX)){
                        V[0xF] = 1;
                    }else{
                        V[0xF] = 0;
                    }
                    pc += 2;
                    break;
                case 0x0005:
                     if(VY > VX)
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    VX -= VY;
                    pc += 2;
                    break;
                case 0x0006:
                    // 8XY6 - Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    V[(0xF)] = VX & 0x1;
                    VX >>= 1;
                    pc += 2; 
                    break;
                case 0x0007:
                    if(VX > VY)   // VY-VX
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    VX = VY - VX;
                    pc += 2;
                    break;
                case 0x000E:
                    // 8XYE - Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    V[0xF] = VX >> 7;  //as it is a 16-bit register
                    VX <<= 1;
                    pc += 2;
                    break;
                default:
                    printf("Unknown opcode\n");    
                    exit(3);
            }
            break;
        case 0x9000:
            //Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block) 
            if(VX != VY){
                pc += 4;
            }else{
                pc += 2;
            }
            break;
        case 0xA000:
            //Sets I to the address NNN. 
            I = NNN;
            pc += 2;
            break;
        case 0xB000:
            //Jumps to the address NNN plus V0. 
            pc = NNN + V[0x0];
            break;
        case 0xC000:
            //Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. 
            VX = (rand() % (0xFF + 1)) & NN;
            pc += 2;
            break;
       case 0xD000:
        {
            unsigned short x = VX;
            unsigned short y = VY;
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(graphics[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            V[0xF] = 1;
                        }
                        graphics[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
        }
            break;

        case 0xE000:
            switch(opcode & 0x00FF){
                case 0x009E:
                    if(keys[VX] != 0){
                        pc += 4;
                    }else{
                        pc += 2;
                    }
                    break;
                case 0x00A1:
                    if(keys[VX] == 0){
                        pc += 4;
                    }else{
                        pc += 2;
                    }
                    break;
                default:
                    printf("unknown opcode \n");    
            }
            break;
        case 0xF000:
            switch(opcode & 0x00FF){
                case 0x0007:
                    // FX07 Sets VX to the value of the delay timer. 
                    VX = delayTimer;
                    pc += 2;
                    break;
                case 0x000A:
                {
                    // FX0A A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event) 
                   bool key_pressed = false;

                    for(int i = 0; i < 16; ++i)
                    {
                        if(keys[i] != 0)
                        {
                            VX = i;
                            key_pressed = true;
                        }
                    }

                    // If no key is pressed, return and try again.
                    if(!key_pressed)
                        return;

                    pc += 2;
                }
                    break;
                case 0x0015:
                    //FX15 Sets the delay timer to VX. 
                    delayTimer = VX;
                    pc += 2;
                    break;
                case 0x0018:
                    //FX18 Sets the sound timer to VX. 
                    soundTimer = VX;
                    pc += 2;
                    break;
                case 0x001E:
                    //FX1E Adds VX to I. VF is set to 1 when there is a range overflow (I+VX>0xFFF), and to 0 when there isn't
                    V[0xF] = (I + VX) > 0xFFF;
                    I += VX;
                    pc += 2; 
                    break;
                case 0x0029:
                    //FX29 Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. 
                    I = VX * 0x5;
                    pc += 2;
                    break;
                case 0x0033:
                    //FX33 Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I,  
                    // the middle digit at I plus 1, and the least significant digit at I plus 2. 
                    //(In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
                    memory[I] = VX / 100;
                    memory[I + 1] = (VX / 10) % 10;
                    memory[I + 2] = VX % 10;
                    pc += 2;
                    break;
                case 0x0055:
                    //FX55 Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
                    for(int i = 0; i <= X; i++){
                        memory[I + i] = V[i];
                    }

                    // On the original interpreter,
                    // when the operation is done, I = I + X + 1.
                    //I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;  
                case 0x0065:
                    // FX65 Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                    for(int i = 0; i <= X; i++){
                        V[i] = memory[I + i];
                    }

                    // On the original interpreter,
                    // when the operation is done, I = I + X + 1.
                    //I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
            }
            break;

        default:
            printf("\nUnimplemented op code: %.4X\n", opcode);
            exit(3);
    }
    // Update timers
    if (delayTimer > 0)
        --delayTimer;

    if (soundTimer > 0)
        if(soundTimer == 1);
        --soundTimer;
}
