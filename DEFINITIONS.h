#define VX (V[(opcode & 0x0F00) >> 8])

#define VY (V[(opcode & 0x00F0) >> 4])

#define X ((opcode & 0x0F00) >> 8)

#define Y ((opcode & 0x00F0) >> 4)

#define NNN (opcode & 0x0FFF)

#define NN (opcode & 0x00FF)