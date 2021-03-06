#include "chip8.h"

Chip8::Chip8() {}
Chip8::~Chip8() {}

void Chip8::initialize()
{
    pc = 0x200; // Program counter starts at 0x200
    opcode = 0; // Reset current opcode
    I = 0;      // Reset index register
    sp = 0;     // Reset stack pointer

    // Clear display
    for (int i = 0; i < 2048; ++i)
        gfx[i] = 0;

    // Clear stack
    for (int i = 0; i < 16; ++i)
        stack[i] = 0;

    // Clear keys and registers V0-VF
    for (int i = 0; i < 16; ++i)
        key[i] = V[i] = 0;

    // Clear memory
    for (int i = 0; i < 4096; ++i)
        memory[i] = 0;

    // Load fontset
    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;

    drawFlag = false;
}

void Chip8::loadGame(string fileName)
{
    ifstream romFile(fileName, ios::binary | ios::ate);
    streampos pos = romFile.tellg();

    vector<char> buffer(pos);

    romFile.seekg(0, ios::beg);
    romFile.read(&buffer[0], pos);

    for (int i = 0; i < buffer.size(); ++i)
        memory[i + 512] = buffer[i];
}

void Chip8::emulateCycle()
{
    // Fetch Opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Execute opcode
    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // 0x00E0: Clears the screen
            for (int i = 0; i < 2048; ++i)
                gfx[i] = 0;
            pc += 2;
            break;

        case 0x000E: // 0x00EE: Returns from subroutine
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
        }
        break;
    case 0x1000: // 1NNN: Jumps to address NNN.
        pc = opcode & 0x0FFF;
        break;
    case 0x2000: // 2NNN: Calls subroutine at NNN.
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;
        break;
    case 0x3000: // 3XNN: Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;
    case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;
    case 0x5000: // 5XY0: Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;
    case 0x6000: // 6XNN: Sets VX to NN.
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;
    case 0x7000: // 7XNN: Adds NN to VX. (Carry flag is not changed)
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // 8XY0: Sets VX to the value of VY.
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0001: // 8XY1: Sets VX to VX or VY. (Bitwise OR operation)
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0002: // 8XY2: Sets VX to VX and VY. (Bitwise AND operation)
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0003: // 8XY3: Sets VX to VX xor VY.
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; //carry
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
            if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
                V[0xF] = 0; //borrow
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        case 0x0006: // 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;
        case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
            if (V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8])
                V[0xF] = 0; //borrow
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x000E: // 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;
        }
        break;
    case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;
    case 0xA000: // ANNN: Sets I to the address NNN.
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    case 0xB000: // BNNN: Jumps to the address NNN plus V0.
        pc = V[0] + (opcode & 0x0FFF);
        break;
    case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        V[(opcode & 0x0F00) >> 8] = ((rand() % 255) & (opcode & 0x00FF));
        pc += 2;
        break;
    case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels.
    {
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                        V[0xF] = 1;
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }

        drawFlag = true;
        pc += 2;
    }
    break;

    case 0xE000:
        switch (opcode & 0x00FF)
        {
        // EX9E: Skips the next instruction
        // if the key stored in VX is pressed
        case 0x009E:
            if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                pc += 4;
            else
                pc += 2;
            break;
        case 0x00A1:
            if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                pc += 4;
            else
                pc += 2;
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007: // FX07: Sets VX to the value of the delay timer.
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;
        case 0x000A: // FX0A: A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
        {
            bool keyPress = false;
            for (int i = 0; i < 16; ++i)
            {
                if (key[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyPress = true;
                }
            }
            if (keyPress)
                pc += 2;
        }
        break;
        case 0x0015: // FX15: Sets the delay timer to VX.
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0018: // FX18: Sets the sound timer to VX.
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x001E: // FX1E: Adds VX to I. VF is not affected.
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            I = V[(opcode & 0x0F00) >> 8] * 5;
            pc += 2;
            break;
        case 0x0033:                                               // FX33: Stores the binary-coded decimal representation of VX at address I in memory.
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;           // Most significant digit
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10; // Middle digit
            memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;        // Least significant digit
            pc += 2;
            break;
        case 0x0055: // FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
        {            //TODO FX55 and FX65 check footnotes for "I itself is left unmodified"
            int x = (opcode & 0x0F00) >> 8;
            for (int i = 0; i <= x; ++i)
            {
                memory[I + i] = V[i];
            }
            pc += 2;
        }
        break;
        case 0x0065: // FX65: Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
        {
            int x = (opcode & 0x0F00) >> 8;
            for (int i = 0; i <= x; ++i)
            {
                V[i] = memory[I + i];
            }
            pc += 2;
        }
        break;
        }
        break;
    default:
        printf("Unknown opcode: 0x%X\n", opcode);
        exit(0);
    }

    // Update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }
}
