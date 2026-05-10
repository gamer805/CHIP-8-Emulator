#include "opcodes.h"

// clear screen
void Opcode00E0(WORD opcode) {
    memset(m_ScreenData, 0, sizeof(m_ScreenData));
}

// return
void Opcode00EE(WORD opcode) {
    --stack_pointer;
    m_ProgramCounter = m_Stack[stack_pointer];
}

// jump to NNN
void Opcode1NNN(WORD opcode) {
    m_ProgramCounter = opcode & 0x0FFF;
}

// call subroutine at NNN
void Opcode2NNN(WORD opcode) {
    m_Stack.push_back(m_ProgramCounter);
    m_ProgramCounter = opcode & 0x0FFF;
}

// skip next instruction if Vx == NN
void Opcode3XNN(WORD opcode) {
    BYTE X = (opcode >> 8) & 0x0F;
    BYTE NN = opcode & 0x00FF;
    if (m_Registers[X] == NN) {
        m_ProgramCounter += 2;
    }
}

// skip next instruction if Vx != NN
void Opcode4XNN(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE NN = opcode & 0x00FF;
    if (m_Registers[X] != NN) {
        m_ProgramCounter += 2;
    }
}

// skip next instruction if Vx == Vy
void Opcode5XY0(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    if (m_Registers[X] == m_Registers[Y]) {
        m_ProgramCounter += 2;
    }
}

// set Vx to NN
void Opcode6XNN(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE NN = opcode & 0x00FF;
    m_Registers[X] = NN;
}

void Opcode7XNN(WORD opcode);
void Opcode8XY0(WORD opcode);
void Opcode8XY1(WORD opcode);
void Opcode8XY2(WORD opcode);
void Opcode8XY3(WORD opcode);
void Opcode8XY4(WORD opcode);
void Opcode8XY5(WORD opcode);
void Opcode8XY6(WORD opcode);
void Opcode8XY7(WORD opcode);
void Opcode8XYE(WORD opcode);
void Opcode9XY0(WORD opcode);
void OpcodeANNN(WORD opcode);
void OpcodeBNNN(WORD opcode);
void OpcodeCXNN(WORD opcode);
void OpcodeDXYN(WORD opcode);
void OpcodeEX9E(WORD opcode);
void OpcodeEXA1(WORD opcode);
void OpcodeFX07(WORD opcode);
void OpcodeFX0A(WORD opcode);
void OpcodeFX15(WORD opcode);
void OpcodeFX18(WORD opcode);
void OpcodeFX1E(WORD opcode);
void OpcodeFX29(WORD opcode);
void OpcodeFX33(WORD opcode);
void OpcodeFX55(WORD opcode);
void OpcodeFX65(WORD opcode);