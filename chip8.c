#include "chip8.h"

unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void clear_display(chip8 *c)
{
    for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
        c->display[i] = 0;
    }
}

void initialize(chip8 *c)
{
    c->PC = 0x200; // programs start at 0x200
    c->opcode = 0; // clear the opcode
    c->I      = 0; // clear the index register
    c->SP     = 0; // clear the stack pointer
    c->pause  = 0;

    clear_display(c);
    
    for (int i = 0; i < 16; i++) {
        c->stack[i] = 0;  // Clear stack
        c->V[i] = 0;      // Clear registers V0-VF
    }

    // Clear memory
    for (int i = 0; i < 4096; i++) {
        c->memory[i] = 0;
    }

    // Fontset data is stored at 0x50
    for (int i = 0; i < 80; i++) {
        c->memory[i + 0x50] = fontset[i];
    }

    // Reset timers
    c->DT = 0;
    c->ST = 0;
}

/*
TODO:
-----

Dxyn - DRW Vx, Vy, nibble
*/
void execute(chip8 *c)
{
    // Merge two bytes to form a full instruction
    c->opcode = c->memory[c->PC] << 8 | c->memory[c->PC + 1];

    switch (c->opcode & 0xF000) {
        // 00E0 - CLS or 00EE - RET
        case (0x0000):
        {
            if (c->opcode & 0xFFFF == 0x00E0) {
                // Clear the display
                clear_display(c);
                c->PC += 2;
            } else {
                // RET
                c->PC = c->stack[c->SP];
                c->SP--;
            }
            break;
        }
        // 1nnn - JP addr
        case (0x1000):
        {
            c->PC = c->opcode & 0x0FFF;
            break;
        }
        // 2nnn - CALL addr
        case (0x2000):
        {
            c->stack[c->SP] = c->PC;
            c->SP++;

            if (c->SP > 15) {
                fprintf(stderr, "Stack overflow\n");
                exit(1);
            }
            c->PC += 2;
            break;
        }
        // 3xkk - SE Vx, byte
        case (0x3000):
        {
            if (c->V[c->opcode & 0x0F00 >> 8] == c->opcode & 0x00FF) {
                c->PC += 2;
            }
            c->PC += 2;
            break;
        }
        // 4xkk - SNE Vx, byte
        case (0x4000):
        {
            if (c->V[c->opcode & 0x0F00 >> 8] != c->opcode & 0x00FF) {
                c->PC += 2;
            }
            c->PC += 2;
            break;
        }
        // 5xy0 - SE Vx, Vy
        case (0x5000):
        {
            if (c->V[c->opcode & 0x0F00 >> 8] == c->V[c->opcode & 0x00F0 >> 4]) {
                c->PC += 2;
            }
            c->PC += 2;
            break;
        }
        // 6xkk - LD Vx, byte
        case (0x6000):
        {
            c->V[c->opcode & 0x0F00 >> 8] = c->opcode & 0x00FF;
            c->PC += 2;
            break;
        }
        // 7xkk - ADD Vx, byte
        case (0x7000):
        {
            c->V[c->opcode & 0x0F00 >> 8] += c->opcode & 0x00FF;
            c->PC += 2;
            break;
        }
        case (0x8000):
        {
            // 8xy0 - LD Vx, Vy
            if (c->opcode & 0xF00F == 0x8000) {
                c->V[c->opcode & 0x0F00 >> 8] = c->V[c->opcode & 0x00F0 >> 4];
            // 8xy1 - OR Vx, Vy
            } else if (c->opcode & 0xF00F == 0x8001) {
                c->V[c->opcode & 0x0F00 >> 8] |= c->V[c->opcode & 0x00F0 >> 4];
            // 8xy2 - AND Vx, Vy
            } else if (c->opcode & 0xF00F == 0x8002) {
                c->V[c->opcode & 0x0F00 >> 8] &= c->V[c->opcode & 0x00F0 >> 4];
            // 8xy3 - XOR Vx, Vy
            } else if (c->opcode & 0xF00F == 0x8003) {
                c->V[c->opcode & 0x0F00 >> 8] ^= c->V[c->opcode & 0x00F0 >> 4];
            // 8xy4 - ADD Vx, Vy
            } else if (c->opcode & 0xF00F == 0x8004) {
                short short_sum = c->V[c->opcode & 0x0F00 >> 8] + c->V[c->opcode & 0x00F0 >> 4];

                if (short_sum > 255) {
                    c->V[15] = 1;
                } else {
                    c->V[15] = 0;
                }

                c->V[c->opcode & 0x0F00 >> 8] = short_sum & 0xFF;
            // 8xy5 - SUB Vx, Vy
            } else if (c->opcode & 0xF00F == 0x8005) {
                if (c->V[c->opcode & 0x0F00 >> 8] > c->V[c->opcode & 0x00F0 >> 4]) {
                    c->V[15] = 1;
                } else {
                    c->V[15] = 0;
                }

                c->V[c->opcode & 0x0F00 >> 8] -= c->V[c->opcode & 0x00F0 >> 4];
            // 8xy6 - SHR Vx {, Vy}
            } else if (c->opcode & 0xF00F == 0x8006) {
                if (c->V[c->opcode & 0x0F00 >> 8] & 0x01 == 1) {
                    c->V[15] = 1;
                } else {
                    c->V[15] = 0;
                }

                c->V[c->opcode & 0x00F0 >> 4] = c->V[c->opcode & 0x00F0 >> 4] >> 1;
                c->V[c->opcode & 0x0F00 >> 8] = c->V[c->opcode & 0x00F0 >> 4];
            // 8xy7 - SUBN Vx, Vy
            } else if (c->opcode & 0xF00F == 0x8007) {
                if (c->V[c->opcode & 0x0F00 >> 8] < c->V[c->opcode & 0x00F0 >> 4]) {
                    c->V[15] = 1;
                } else {
                    c->V[15] = 0;
                }
                c->V[c->opcode & 0x0F00 >> 8] = c->V[c->opcode & 0x00F0 >> 4] - c->V[c->opcode & 0x0F00 >> 8];
            // 8xyE - SHL Vx {, Vy}
            } else if (c->opcode & 0xF00F == 0x800E) {
                if (c->V[c->opcode & 0x0F00 >> 8] & 0x80 == 1) {
                    c->V[15] = 1;
                } else {
                    c->V[15] = 0;
                }

                c->V[c->opcode & 0x00F0 >> 4] = c->V[c->opcode & 0x00F0 >> 4] << 1;
                c->V[c->opcode & 0x0F00 >> 8] = c->V[c->opcode & 0x00F0 >> 4];
            }

            c->PC += 2;
            break;
        }
        // 9xy0 - SNE Vx, Vy
        case (0x9000):
        {
            if (c->V[c->opcode & 0x0F00 >> 8] != c->V[c->opcode & 0x00F0 >> 4]) {
                c->PC += 2;
            }

            c->PC += 2;
            break;
        }
        // Annn - LD I, addr
        case (0xA000):
        {
            c->I = c->opcode & 0x0FFF;
            c->PC += 2;
            break;
        }
        // Bnnn - JP V0, addr
        case (0xB000):
        {
            c->PC = c->V[0] + c->opcode & 0x0FFF;
            break;
        }
        // Cxkk - RND Vx, byte
        case (0xC000):
        {
            c->V[c->opcode & 0x0F00 >> 8] = (rand() % 256) & c->opcode & 0x00FF;
            c->PC += 2;
            break;
        }
        // Dxyn - DRW Vx, Vy, nibble
        case (0xD000):
        {
            int x  = c->V[c->opcode & 0x0F00 >> 8];
            int y  = c->V[c->opcode & 0x00F0];
            char h = c->opcode & 0x000F;

            for (int draw_y = y; draw_y < y + h; draw_y++) {
                char data = c->memory[c->I + draw_y];
                for (int draw_x = x; x < x + 8; draw_x++) {
                    char bit_value = data & (0x80 >> draw_x);
                    if (bit_value) {
                        if (c->display[x + draw_x + (y + draw_y) * W_HEIGHT]) {
                            c->V[15] = 1;
                        } else {
                            c->V[15] = 0;
                        }
                        c->display[x + draw_x + (y + draw_y) * W_HEIGHT] ^= bit_value;
                    }
                }
            }

            c->PC += 2;
            break;
        }
        case (0xE000):
        {
            // Ex9E - SKP Vx
            if (c->opcode & 0xF0FF == 0xE09E && c->keys[c->V[c->opcode & 0x0F00 >> 8]]) {
                c->PC += 2;
            // ExA1 - SKNP Vx
            } else if (c->opcode & 0xF0FF == 0xE0A1 && !c->keys[c->V[c->opcode & 0x0F00 >> 8]]) {
                c->PC += 2;
            }

            c->PC += 2;
            break;
        }
        case (0xF000):
        {
            // Fx07 - LD Vx, DT
            if (c->opcode & 0xF0FF == 0xF007) {
                c->V[c->opcode & 0x0F00 >> 8] = c->DT;

                c->PC += 2;
            // Fx0A - LD Vx, K
            } else if (c->opcode & 0xF0FF == 0xF00A) {
                // Wait for a key press, store the value of the key in Vx.
                // All execution stops until a key is pressed, then the value of that key is stored in Vx.
                c->pause = 1;

                for (int i = 0; i < 16; i++) {
                    if (c->keys[i]) {
                        c->pause = 0;
                        c->V[c->opcode & 0x0F00 >> 8] = i;
                        break;
                    }
                }

                if (!c->pause) {
                    c->PC += 2;
                }
            // Fx15 - LD DT, Vx
            } else if (c->opcode & 0xF015 == 0xF015) {
                c->DT = c->V[c->opcode & 0x0F00 >> 8];

                c->PC += 2;
            // Fx18 - LD ST, Vx
            } else if (c->opcode & 0xF018 == 0xF018) {
                c->ST = c->V[c->opcode & 0x0F00 >> 8];

                c->PC += 2;
            // Fx1E - ADD I, Vx
            } else if (c->opcode & 0xF01E == 0xF01E) {
                c->memory[c->I] += c->V[c->opcode & 0x0F00 >> 8];

                c->PC += 2;
            // Fx29 - LD F, Vx
            } else if (c->opcode & 0xF029 == 0xF029) {
                c->I = 0x50 + (c->opcode & 0x0F00) >> 8 * 5;

                c->PC += 2;
            // Fx33 - LD B, Vx
            } else if (c->opcode & 0xF033 == 0xF033) {
                int n = c->V[c->opcode & 0x0F00 >> 8];
                char h = (n / 100) % 10, t = (n / 10) % 10, o = n % 10; 

                c->memory[c->I] = h;
                c->memory[c->I+1] = t;
                c->memory[c->I+2] = o;

                c->PC += 2;
            // Fx55 - LD [I], Vx
            } else if (c->opcode & 0xF055 == 0xF055) {
                for (int i = 0; i < (c->opcode & 0x0F00 >> 8); i++) {
                    c->memory[c->I + i] = c->V[i];
                }

                c->PC += 2;
            // Fx65 - LD Vx, [I]
            } else if (c->opcode & 0xF065 == 0xF065) {
                for (int i = 0; i < (c->opcode & 0x0F00 >> 8); i++) {
                    c->V[i] = c->memory[c->I + i];
                }

                c->PC += 2;
            }

            break;
        }
        default:
        {
            fprintf(stderr, "Invalid opcode %i at %i\n", c->opcode, c->PC);
            exit(1);
            break;
        }
    }

    // Update timers
    if (c->DT > 0 && SDL_GetTicks() % 16 == 0) {
        c->DT--;
    } else {
        c->DT = 0;
    }

    if (c->ST > 0 && SDL_GetTicks() % 16 == 0) {
        fprintf(stdout, "\aBeep!\n");
        c->ST--;
    } else {
        c->ST = 0;
    }
}

void load_file(chip8 *c, const char *filename)
{
    FILE *fp;

    fp = fopen(filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "invalid filename\n");
        exit(1);
    }

    int temp = 0x200;
    short in[8];
    while (fscanf(fp, "%c %c %c %c %c %c %c %c\n",
      in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7]) != EOF) {
        for (int i = 0; i < 8; i++) {
            char high = in[i] & 0xFF00 >> 4;

            c->memory[temp] = high;

            char low  = in[i] & 0xFF;

            c->memory[temp+2] = low;

            temp += 4;
        }
    }

    fclose(fp);
}