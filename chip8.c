#include "chip8.h"

/* Main operations */
void clear_display(chip8 *c)
{
    for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
        c->display[i] = 0;
    }
}

void initialize(chip8 *c);

/* possible idea: modulate the execution loop by 
 *                creating a method for each instruction.
 *
 *                that way the switch case won't be taking
 *                up the entire screen.
 */
void execute_instruction(chip8 *c)
{
    c->opcode = c->memory[c->PC] << 8 | c->memory[c->PC + 1];

    switch(c->opcode & 0xF000) {
        case 0x0000: {
            // 00E0 - clear the display
            if (c->opcode & 0xFFFF == 0x00E0) {
                clear_display(c);
            }
            // 00EE - return from a subroutine
            else if (c->opcode & 0xFFFF == 0x00EE) {
                set_pc(c, get_stack_top(c));
                stack_pop(c);
            }
            break;
        }
        // 1nnn - jump to nnn
        case 0x1000: {
            set_pc(c, get_opcode_nnn(c));
            break;
        }
        // 2nnn - call subroutine at nnn
        case 0x2000: {
            stack_push(c, get_pc(c));
            set_pc(c, get_opcode_nnn(c));
            break;
        }
        // 3xkk - skip next instruction if Vx = kk
        case 0x3000: {
            if (get_reg_value(c, get_opcode_x(c)) == get_opcode_kk(c)) {
                pc_increment(c);
            }
            break;
        }
        // 4xkk - skip next instruction if Vx != kk
        case 0x4000: {
            if (get_reg_value(c, get_opcode_x(c)) != get_opcode_kk(c)) {
                pc_increment(c);
            }
            break;
        }
        // 5xy0 - skip next instruction if Vx = Vy
        case 0x5000: {
            if (get_reg_value(c, get_opcode_x(c)) == get_reg_value(c, get_opcode_y(c))) {
                pc_increment(c);
            }
            break;
        }
        // 6xkk - puts the value kk into register Vx
        case 0x6000: {
            set_reg_value(c, get_opcode_x(c), get_opcode_kk(c));
            break;
        }
        // 7xy0 - adds Vx and Vy and stores it in Vx
        case 0x7000: {
            char a = get_reg_value(c, get_opcode_x(c));
            char b = get_reg_value(c, get_opcode_y(c));

            set_reg_value(c, a, (a + b) % 256);
            break;
        }
        case 0x8000: {
            unsigned char a = get_reg_value(c, get_opcode_x(c));
            unsigned char b = get_reg_value(c, get_opcode_y(c));

            // 8xy0 - load Vy into Vx, Vx = Vy
            if (test_opcode(c, 0x000F, 0)) {
                set_reg_value(c, get_opcode_x(c), b);
            }
            // 8xy1 - bitwise OR, Vx = Vx | Vy
            else if (test_opcode(c, 0x000F, 1)) {
                set_reg_value(c, get_opcode_x(c), a | b);
            }
            // 8xy2 - bitwise AND, Vx = Vx & Vy
            else if (test_opcode(c, 0x000F, 2)) {
                set_reg_value(c, get_opcode_x(c), a & b);
            }
            // 8xy3 - bitwise XOR, Vx = Vx ^ Vy
            else if (test_opcode(c, 0x000F, 3)) {
                set_reg_value(c, get_opcode_x(c), a ^ b);
            }
            // 8xy4 - bitwise ADD, Vx = Vx + Vy, VF = carry
            else if (test_opcode(c, 0x000F, 4)) {
                if (a + b >= 256) {
                    set_reg_value(c, 0xF, 1);
                } else {
                    set_reg_value(c, 0xF, 0);
                }
                set_reg_value(c, get_opcode_x(c), (a + b) % 256);
            }
            // 8xy5 - bitwise SUB, Vx = Vx - Vy, VF = not borrow
            else if (test_opcode(c, 0x000F, 5)) {
                // check the carry flag
                if (a > b) {
                    set_reg_value(c, 0xF, 1);
                } else {
                    set_reg_value(c, 0xF, 0);
                }
                set_reg_value(c, get_opcode_x(c), a - b); // set Vx = Vx - Vy
            }
            // 8xy6 - bitwise SHR, Vx = Vx >> 1, VF = 1 if LSB is 1
            else if (test_opcode(c, 0x000F, 6)) {
                if (a & 0x1 == 1) {
                    set_reg_value(c, 0xF, 1);
                } else {
                    set_reg_value(c, 0xF, 0);
                }

                set_reg_value(c, get_opcode_x(c), a >> 1);
            }
            // 8xy7 - bitwise SUBN, Vx = Vy - Vx, VF = 1 if Vy > Vx
            else if (test_opcode(c, 0x000F, 7)) {
                if (b > a) {
                    set_reg_value(c, 0xF, 1);
                } else {
                    set_reg_value(c, 0xF, 0);
                }

                set_reg_value(c, get_opcode_x(c), b - a);
            }
            // 8xyE - bitwise SHL, Vx = Vx << 1, VF = 1 if MSB is 1
            else if (test_opcode(c, 0x000F, 0xE)) {
                if (a & 0x80 == 1) {
                    set_reg_value(c, 0x000F, 1);
                } else {
                    set_reg_value(c, 0x000F, 0);
                }

                set_reg_value(c, get_opcode_x(c), a << 1);
            }

            break;
        }
        // 9xy0 - skip next instruction if Vx != Vy
        case 0x9000: {
            if (get_reg_value(c, get_opcode_x(c)) != get_reg_value(c, get_opcode_y(c))) {
                pc_increment(c);
            }

            break;
        }
        // Annn - load nnn into register I
        case 0xA000: {
            set_addr(c, get_opcode_nnn(c));

            break;
        }
        // Bnnn - jump to location nnn + V0
        case 0xB000: {
            set_pc(c, (get_opcode_nnn(c) + get_reg_value(c, 0)) % MAX_MEMORY);

            break;
        }
        // Cxkk - random byte bitwise AND, Vx = Vx AND rand_byte
        case 0xC000: {
            char n = rand() % 256;
            set_reg_value(c, get_opcode_x(c), get_opcode_x(c) & n);

            break;
        }
        /*
        Dxyn - DRW Vx, Vy, nibble
        Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

        The interpreter reads n bytes from memory, starting at the address stored in I. These bytes
        are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the
        existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set
        to 0. If the sprite is positioned so part of it is outside the coordinates of the display,
        it wraps around to the opposite side of the screen.
        */
        case 0xD000: {
            char n = c->opcode & 0xF;
            char x = get_reg_value(c, get_opcode_x(c));
            char y = get_reg_value(c, get_opcode_y(c));

            char pos_x[8] = {
                 x,
                (x + 1) % W_WIDTH,
                (x + 2) % W_WIDTH,
                (x + 3) % W_WIDTH,
                (x + 4) % W_WIDTH,
                (x + 5) % W_WIDTH,
                (x + 6) % W_WIDTH,
                (x + 7) % W_WIDTH
            };

            // iterate over n-bytes of the sprite in memory
            for (int i = get_addr(c); i < get_addr(c) + n; i++) {
                char sprite_data = c->memory[i];
                bool collision = 0;

                // the width of a fontset sprite is always 8 bits in Chip-8
                collision = set_display_value(c, pos_x[0], y, sprite_data & 0x80 >> 7);
                collision = set_display_value(c, pos_x[1], y, sprite_data & 0x40 >> 6);
                collision = set_display_value(c, pos_x[2], y, sprite_data & 0x20 >> 5);
                collision = set_display_value(c, pos_x[3], y, sprite_data & 0x10 >> 4);
                collision = set_display_value(c, pos_x[4], y, sprite_data & 0x08 >> 3);
                collision = set_display_value(c, pos_x[5], y, sprite_data & 0x04 >> 2);
                collision = set_display_value(c, pos_x[6], y, sprite_data & 0x02 >> 1);
                collision = set_display_value(c, pos_x[7], y, sprite_data & 0x01);

                set_reg_value(c, 0xF, collision);

                y = (y + 1) % W_HEIGHT;
            }

            break;
        }
        case 0xE000: {
            // Ex9E - skip the next instruction if the key with value of Vx is pressed
            char a = get_reg_value(c, get_opcode_x(c));
            if (test_opcode(c, 0x00FF, 0x009E) && get_key_value(c, a)) {
                pc_increment(c);
            }
            // ExA1 - skip the next instruction if the key with value of Vx is not pressed
            else if (test_opcode(c, 0x00FF, 0x00A1) && !get_key_value(c, a)) {
                pc_increment(c);
            }

            break;
        }
        case 0xF000: {
            // Fx07 - value of DT is placed into Vx
            if (test_opcode(c, 0x000F, 0x0007)) {
                set_reg_value(c, get_opcode_x(c), get_dt(c));
            }
            // Fx0A - all execution stops until a key is pressed and the value of
            //        the key is stored in Vx
            else if (test_opcode(c, 0x000F, 0x000A)) {
                c->pause = 1;

                /* TODO: finish this instruction */
            }
            // Fx15 - set delay timer to the value of Vx
            else if (test_opcode(c, 0x00FF, 0x0015)) {
                set_dt(c, get_reg_value(c, get_opcode_x(c)));
            }
            // Fx18 - set sound timer to the value of Vx
            else if (test_opcode(c, 0x00FF, 0x0018)) {
                set_st(c, get_reg_value(c, get_opcode_x(c)));
            }
            // Fx1E - set register I to I + Vx
            else if (test_opcode(c, 0x00FF, 0x001E)) {
                short sum = get_reg_value(c, get_opcode_x(c)) + get_addr(c);

                if (sum >= MAX_MEMORY) {
                    sum = (sum % MAX_MEMORY) + 0x200;
                }

                set_addr(c, sum);
            }
            // Fx29 - load the memory location of sprite Vx into I
            else if (test_opcode(c, 0x00FF, 0x0029)) {
                // fontset starts at 0x50, translate that position by the value of Vx
                // multiplied by the sprite width (5 bytes)
                set_addr( c, 0x50 + get_reg_value(c, get_opcode_x(c)) * 5 );
            }
            // Fx33 - store BCD representation of Vx in I, I + 1, I + 2
            else if (test_opcode(c, 0x00FF, 0x0033)) {
                char value = get_reg_value(c, get_opcode_x(c));

                c->memory[get_addr(c)]     = (value / 100) % 10;
                c->memory[get_addr(c) + 1] = (value / 10) % 10;
                c->memory[get_addr(c) + 2] = (value) % 10;
            }
            // Fx55 - store the values from V0 to Vx starting at memory address I
            else if (test_opcode(c, 0x00FF, 0x0055)) {
                for (int i = 0; i <= get_opcode_x(c); i++) {
                    c->memory[get_addr(c) + i] = get_reg_value(c, i);
                }
            }
            // Fx65 - read registers V0 to Vx starting at memory location I
            else if (test_opcode(c, 0x00FF, 0x0065)) {
                for (int i = 0; i <= get_opcode_x(c); i++) {
                    set_reg_value(c, i, c->memory[get_addr(c) + i]);
                }
            }
            break;
        }
    }

    pc_increment(c);

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
void load_file(chip8 *c, const char *s);

/* Getters */
unsigned char  get_reg_value(chip8 *c, int i)
{
    return c->V[i]; 
}

unsigned char  get_addr(chip8 *c)
{
    return c->I;
}

unsigned char  get_addr_value(chip8 *c)
{
    return c->memory[get_addr(c)];
}

unsigned short get_pc(chip8 *c)
{
    return c->PC;
}

unsigned short get_stack_top(chip8 *c)
{
    return c->stack[c->SP];
}

unsigned char  get_display_value(chip8 *c, int x, int y)
{
    return c->display[x + y * W_WIDTH];
}

unsigned char *get_keys(chip8 *c)
{
    return c->keys;
}

unsigned char  get_key_value(chip8 *c, int i)
{
    return c->keys[i];
}

unsigned short get_opcode(chip8 *c)
{
    return c->opcode;
}

unsigned short get_opcode_nnn(chip8 *c)
{
    return c->opcode & 0x0FFF;
}

unsigned char  get_opcode_x(chip8 *c)
{
    return c->opcode & 0x0F00 >> 8;
}

unsigned char  get_opcode_y(chip8 *c)
{
    return c->opcode & 0x00F0 >> 4;
}

unsigned char  get_opcode_kk(chip8 *c)
{
    return c->opcode & 0x00FF;
}

unsigned char  get_dt(chip8 *c)
{
    return c->DT;
}

unsigned char  get_st(chip8 *c)
{
    return c->ST;
}

unsigned char  get_display_value(chip8 *c, int x, int y)
{
    return c->display[x + (y * W_WIDTH)];
}

bool test_opcode(chip8 *c, int bitmask, int value)
{
    return (c->opcode & bitmask) == value;
}

/* Setters */
void set_reg_value(chip8 *c, int i, char n)
{
    c->V[i] = n;
}

void set_addr_value(chip8 *c, char n)
{
    c->memory[get_addr(c)] = n;
}

void set_addr(chip8 *c, short i)
{
    c->I = i;
}

void set_pc(chip8 *c, short n)
{
    c->PC = n;
}

void pc_increment(chip8 *c)
{
    c->PC += 2;
}

void sp_increment(chip8 *c)
{
    c->SP++;
}

void sp_decrement(chip8 *c)
{
    c->SP--;
}

void stack_pop(chip8 *c)
{
    c->stack[get_sp(c)] = 0;
    sp_decrement(c);
}

void stack_push(chip8 *c, short n)
{
    sp_increment(c);
    c->stack[get_sp(c)] = n;
}

void set_key_value(chip8 *c, int i, char n)
{
    c->keys[i] = n;
}

void set_dt(chip8 *c, char n)
{
    c->DT = n;
}

void set_st(chip8 *c, char n)
{
    c->ST = n;
}

bool set_display_value(chip8 *c, int x, int y, char n)
{
    char temp = get_display_value(c, x, y);
    c->display[x + (y * W_WIDTH)] ^= n;

    if (temp == 1 && get_display_value(c, x, y) == 0) {
        return 1;
    }

    return 0;
}

/*
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
*/
