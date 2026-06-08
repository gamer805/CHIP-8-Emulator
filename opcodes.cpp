#include "opcodes.h"

// clear screen
void Opcode00E0(WORD opcode) {
    memset(m_ScreenData, 0, sizeof(m_ScreenData));
}

// return from subroutine
void Opcode00EE(WORD opcode) {
    m_ProgramCounter = m_Stack.back(); // Get the last address
    m_Stack.pop_back();                // Remove it from the vector
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

// add NN to Vx
void Opcode7XNN(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE NN = opcode & 0x00FF;
    m_Registers[X] += NN;
}

// set Vx = Vy
void Opcode8XY0(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    m_Registers[X] = m_Registers[Y];
}

// bitwise OR: Vx |= Vy
void Opcode8XY1(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    m_Registers[X] |= m_Registers[Y];
}

// bitwise AND: Vx &= Vy
void Opcode8XY2(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    m_Registers[X] &= m_Registers[Y];
}

// bitwise XOR: Vx ^= Vy
void Opcode8XY3(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    m_Registers[X] ^= m_Registers[Y];
}

// add Vx += Vy, set VF to 1 if carry
void Opcode8XY4(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    WORD sum = m_Registers[X] + m_Registers[Y];
    
    m_Registers[0xF] = (sum > 0xFF) ? 1 : 0;
    m_Registers[X] = sum & 0xFF;
}

// subtract Vx -= Vy, set VF to 0 on underflow
void Opcode8XY5(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    
    BYTE flag = (m_Registers[X] >= m_Registers[Y]) ? 1 : 0;
    m_Registers[X] -= m_Registers[Y];
    m_Registers[0xF] = flag;
}

// shift right: Vx >>= 1, VF = LSB prior to shift
void Opcode8XY6(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    
    BYTE flag = m_Registers[X] & 0x1;
    m_Registers[X] >>= 1;
    m_Registers[0xF] = flag;
}

// subtract Vx = Vy - Vx, set VF to 0 on underflow
void Opcode8XY7(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    
    BYTE flag = (m_Registers[Y] >= m_Registers[X]) ? 1 : 0;
    m_Registers[X] = m_Registers[Y] - m_Registers[X];
    m_Registers[0xF] = flag;
}

// shift left: Vx <<= 1, VF = MSB prior to shift
void Opcode8XYE(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    
    BYTE flag = (m_Registers[X] & 0x80) >> 7;
    m_Registers[X] <<= 1;
    m_Registers[0xF] = flag;
}

// skip next instruction if Vx != Vy
void Opcode9XY0(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    if (m_Registers[X] != m_Registers[Y]) {
        m_ProgramCounter += 2;
    }
}

// set I = NNN
void OpcodeANNN(WORD opcode) {
    m_AddressI = opcode & 0x0FFF;
}

// jump to NNN + V0
void OpcodeBNNN(WORD opcode) {
    m_ProgramCounter = m_Registers[0] + (opcode & 0x0FFF);
}

// set Vx = rand() & NN
void OpcodeCXNN(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE NN = opcode & 0x00FF;
    
    m_Registers[X] = (rand() % 256) & NN; 
}

// draw sprite at (Vx, Vy) with height N (Updated for 2D array)
void OpcodeDXYN(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE Y = (opcode & 0x00F0) >> 4;
    BYTE height = opcode & 0x000F;
    
    // Wrap initial coordinates
    BYTE xPos = m_Registers[X] % 64; 
    BYTE yPos = m_Registers[Y] % 32;

    m_Registers[0xF] = 0; // Reset collision flag

    for (unsigned int row = 0; row < height; ++row) {
        // Read sprite data from memory
        BYTE spriteByte = m_GameMemory[m_AddressI + row];

        for (unsigned int col = 0; col < 8; ++col) {
            BYTE spritePixel = spriteByte & (0x80 >> col);
            
            unsigned int screenX = xPos + col;
            unsigned int screenY = yPos + row;

            // Stop drawing this row if it hits the right edge of the screen
            if (screenX >= 64) continue;
            // Stop drawing if it hits the bottom edge of the screen
            if (screenY >= 32) break;

            if (spritePixel) {
                // If the pixel is already on, a collision occurs
                if (m_ScreenData[screenX][screenY] == 1) {
                    m_Registers[0xF] = 1;
                }
                m_ScreenData[screenX][screenY] ^= 1;
            }
        }
    }
}

// skip next instruction if key in Vx is pressed
void OpcodeEX9E(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE key = m_Registers[X] & 0x0F;
    
    // Assumes an extern/global boolean array m_Keys[16]
    if (m_Keys[key]) { 
        m_ProgramCounter += 2;
    }
}

// skip next instruction if key in Vx is NOT pressed
void OpcodeEXA1(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE key = m_Registers[X] & 0x0F;
    
    if (!m_Keys[key]) {
        m_ProgramCounter += 2;
    }
}

// set Vx = delay timer
void OpcodeFX07(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    m_Registers[X] = m_DelayTimer;
}

// wait for key press and store in Vx
void OpcodeFX0A(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    bool keyPress = false;
    
    for (int i = 0; i < 16; ++i) {
        if (m_Keys[i]) {
            m_Registers[X] = i;
            keyPress = true;
            break;
        }
    }
    
    // If no key is pressed, decrement PC to repeat this instruction (blocks execution)
    if (!keyPress) {
        m_ProgramCounter -= 2;
    }
}

// set delay timer = Vx
void OpcodeFX15(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    m_DelayTimer = m_Registers[X];
}

// set sound timer = Vx
void OpcodeFX18(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    m_SoundTimer = m_Registers[X];
}

// I += Vx
void OpcodeFX1E(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    m_AddressI += m_Registers[X];
}

// set I to address of sprite for char in Vx
void OpcodeFX29(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE digit = m_Registers[X] & 0x0F;
    
    // Assuming fontset is loaded into memory starting at 0x050.
    m_AddressI = 0x050 + (digit * 5); 
}

// store BCD representation of Vx in I, I+1, I+2
void OpcodeFX33(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    BYTE value = m_Registers[X];
    
    m_GameMemory[m_AddressI]     = value / 100;
    m_GameMemory[m_AddressI + 1] = (value / 10) % 10;
    m_GameMemory[m_AddressI + 2] = value % 10;
}

// dump registers V0 to Vx into memory starting at I
void OpcodeFX55(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    for (int i = 0; i <= X; ++i) {
        m_GameMemory[m_AddressI + i] = m_Registers[i];
    }
}

// load registers V0 to Vx from memory starting at I
void OpcodeFX65(WORD opcode) {
    BYTE X = (opcode & 0x0F00) >> 8;
    for (int i = 0; i <= X; ++i) {
        m_Registers[i] = m_GameMemory[m_AddressI + i];
    }
}