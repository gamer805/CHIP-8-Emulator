#include <vector>
#include <cstdint>
#include "opcodes.h"



BYTE m_GameMemory[0xFFF];
BYTE m_Registers[16];
WORD m_AddressI;
WORD m_ProgramCounter;
std::vector<WORD> m_Stack;
int stack_pointer;

BYTE m_ScreenData[64][32];

void CPUReset() {
    m_AddressI = 0;
    m_ProgramCounter = 0x200;
    memset(m_Registers, 0, sizeof(m_Registers)); // set all registers to 0

    // load in the game
    FILE *in;
    in = fopen("c:/INVADERS", "rb");
    fread(&m_GameMemory[0x200], 0xfff, 1, in);
    fclose(in);
}

WORD getNextOpcode() {
    WORD res = 0;
    res = m_GameMemory[m_ProgramCounter];
    res <<= 8;
    res |= m_GameMemory[m_ProgramCounter+1];
    m_ProgramCounter += 2;
    return res;
}

void decodeOpcode() {
    WORD opcode = getNextOpcode();
    switch (opcode & 0xF000) {
        case0x1000: Opcode1NNN(opcode); break;
        case0x0000: {
            switch (opcode & 0x000F) {
                case 0x0000: Opcode00E0(opcode); break;
                case 0x000E: Opcode00EE(opcode); break;
            }
        }
    }
}