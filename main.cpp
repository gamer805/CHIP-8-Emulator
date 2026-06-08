#include <vector>
#include <cstdint>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "opcodes.h"
#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#include <libgen.h>
#endif

static std::string resolveRomPath(const char* filename) {
#ifdef __APPLE__
    char exePath[PATH_MAX];
    uint32_t size = sizeof(exePath);
    if (_NSGetExecutablePath(exePath, &size) == 0) {
        char buf[PATH_MAX];
        strncpy(buf, exePath, PATH_MAX - 1);
        buf[PATH_MAX - 1] = '\0';
        std::string resourcePath = std::string(dirname(buf)) + "/../Resources/" + filename;
        char resolved[PATH_MAX];
        if (realpath(resourcePath.c_str(), resolved) != nullptr) {
            return resolved;
        }
    }
#endif

    std::string resourcesPath = std::string("Resources/") + filename;
    std::string parentResourcesPath = std::string("../Resources/") + filename;
    const char* fallbacks[] = {
        filename,
        resourcesPath.c_str(),
        parentResourcesPath.c_str(),
    };
    for (const char* path : fallbacks) {
        FILE* probe = fopen(path, "rb");
        if (probe) {
            fclose(probe);
            return path;
        }
    }
    return filename;
}

BYTE m_GameMemory[0x1000] = {0};
BYTE m_Registers[16] = {0};
WORD m_AddressI = 0;
WORD m_ProgramCounter = 0;
std::vector<WORD> m_Stack;
int stack_pointer = 0;

BYTE m_ScreenData[64][32] = {0};
BYTE m_DelayTimer = 0;
BYTE m_SoundTimer = 0;
bool m_Keys[16] = {false};

void CPUReset() {
    m_AddressI = 0;
    m_ProgramCounter = 0x200;
    memset(m_Registers, 0, sizeof(m_Registers)); // set all registers to 0

    std::string romPath = resolveRomPath("INVADERS");
    FILE* in = fopen(romPath.c_str(), "rb");
    if (!in) {
        std::cerr << "Failed to open ROM: " << romPath << std::endl;
        exit(1);
    }
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
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: Opcode00E0(opcode); break;
                case 0x00EE: Opcode00EE(opcode); break;
            }
            break;
        case 0x1000: Opcode1NNN(opcode); break;
        case 0x2000: Opcode2NNN(opcode); break;
        case 0x3000: Opcode3XNN(opcode); break;
        case 0x4000: Opcode4XNN(opcode); break;
        case 0x5000: Opcode5XY0(opcode); break;
        case 0x6000: Opcode6XNN(opcode); break;
        case 0x7000: Opcode7XNN(opcode); break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: Opcode8XY0(opcode); break;
                case 0x0001: Opcode8XY1(opcode); break;
                case 0x0002: Opcode8XY2(opcode); break;
                case 0x0003: Opcode8XY3(opcode); break;
                case 0x0004: Opcode8XY4(opcode); break;
                case 0x0005: Opcode8XY5(opcode); break;
                case 0x0006: Opcode8XY6(opcode); break;
                case 0x0007: Opcode8XY7(opcode); break;
                case 0x000E: Opcode8XYE(opcode); break;
            }
            break;
        case 0x9000: Opcode9XY0(opcode); break;
        case 0xA000: OpcodeANNN(opcode); break;
        case 0xB000: OpcodeBNNN(opcode); break;
        case 0xC000: OpcodeCXNN(opcode); break;
        case 0xD000: OpcodeDXYN(opcode); break;
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: OpcodeEX9E(opcode); break;
                case 0x00A1: OpcodeEXA1(opcode); break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: OpcodeFX07(opcode); break;
                case 0x000A: OpcodeFX0A(opcode); break;
                case 0x0015: OpcodeFX15(opcode); break;
                case 0x0018: OpcodeFX18(opcode); break;
                case 0x001E: OpcodeFX1E(opcode); break;
                case 0x0029: OpcodeFX29(opcode); break;
                case 0x0033: OpcodeFX33(opcode); break;
                case 0x0055: OpcodeFX55(opcode); break;
                case 0x0065: OpcodeFX65(opcode); break;
            }
            break;
    }
}

int main(int argc, char* args[]) {
    // 1. Initialize Emulator
    CPUReset();
    
    // 2. Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    const int scale = 10;
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        64 * scale, 32 * scale, SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Timing tracking
    auto lastTimerTick = std::chrono::high_resolution_clock::now();
    bool quit = false;
    SDL_Event e;

    // 3. Main Emulation Loop
    while (!quit) {
        // --- INPUT HANDLING ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                bool isPressed = (e.type == SDL_KEYDOWN);
                switch (e.key.keysym.sym) {
                    case SDLK_1: m_Keys[0x1] = isPressed; break;
                    case SDLK_2: m_Keys[0x2] = isPressed; break;
                    case SDLK_3: m_Keys[0x3] = isPressed; break;
                    case SDLK_4: m_Keys[0xC] = isPressed; break;
                    case SDLK_q: m_Keys[0x4] = isPressed; break;
                    case SDLK_w: m_Keys[0x5] = isPressed; break;
                    case SDLK_e: m_Keys[0x6] = isPressed; break;
                    case SDLK_r: m_Keys[0xD] = isPressed; break;
                    case SDLK_a: m_Keys[0x7] = isPressed; break;
                    case SDLK_s: m_Keys[0x8] = isPressed; break;
                    case SDLK_d: m_Keys[0x9] = isPressed; break;
                    case SDLK_f: m_Keys[0xE] = isPressed; break;
                    case SDLK_z: m_Keys[0xA] = isPressed; break;
                    case SDLK_x: m_Keys[0x0] = isPressed; break;
                    case SDLK_c: m_Keys[0xB] = isPressed; break;
                    case SDLK_v: m_Keys[0xF] = isPressed; break;
                }
            }
        }

        // --- CPU EXECUTION ---
        // Run ~10 instructions per frame to simulate roughly 600Hz clock speed
        for (int i = 0; i < 10; ++i) {
            decodeOpcode();
        }

        // --- TIMERS (60Hz) ---
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastTimerTick).count();
        
        if (dt >= 16.67f) { // 1000ms / 60 = 16.67ms
            if (m_DelayTimer > 0) --m_DelayTimer;
            if (m_SoundTimer > 0) {
                --m_SoundTimer;
                // TODO: Insert SDL_Audio code here to play a beep
            }
            lastTimerTick = currentTime;
        }

        // --- RENDERING ---
        // Clear screen to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Set draw color to white for active pixels
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 64; ++x) {
                if (m_ScreenData[x][y] == 1) {
                    SDL_Rect pixelRect = { x * scale, y * scale, scale, scale };
                    SDL_RenderFillRect(renderer, &pixelRect);
                }
            }
        }

        // Update the window
        SDL_RenderPresent(renderer);

        // Cap framerate (approx 60Hz loop)
        SDL_Delay(16); 
    }

    // 4. Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}