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

/* Main operations */
void clear_display(chip8 *c)
{
    for (int i = 0; i < W_WIDTH * W_HEIGHT; i++) {
        c->display[i] = 0;
    }
}

void initialize(chip8 *c)
{
    /* Initialize the memory (4096 bytes) */
    for (int i = 0; i < MAX_MEMORY; i++) {
        c->memory[i] = 0;
    }

    /* clear the opcode */
    c->opcode = 0;

    for (int i = 0; i < 16; i++) {
        c->V[i]     = 0;  /* clear general purpose registers */
        c->stack[i] = 0;  /* clear the stack */
        c->keys[i]  = 0;  /* clear keys */
    }

    /* clear the memory addresses */
    c->I  = 0;
    /* set the program to the beginning of program data */
    c->PC = 0x200;
    /* clear stack pointer */
    c->SP = 0;

    /* clear the timers */
    set_dt(c, 0);
    set_st(c, 0);

    /* clear display */
    clear_display(c);

    /* fontset data is stored at 0x50 */
    for (int i = 0; i < 80; i++) {
        c->memory[i + 0x50] = fontset[i];
    }

    c->pause = 0;
    c->key_flag = -1;
}

void execute_instruction(chip8 *c)
{
    c->opcode = (c->memory[c->PC] << 8) | c->memory[c->PC + 1];

    switch(c->opcode & 0xF000) {
        case 0x0000: {
            /* 00E0 - clear the display */
            if (c->opcode & 0xFFFF == 0x00E0) {
                op_00E0(c);
            }
            /* 00EE - Return from a subroutine 
               Not an oftenly used instruction by modern interpreters
            else if (c->opcode & 0xFFFF == 0x00EE) {
                op_00EE(c);
            }
            */
            break;
        }
        /* 1nnn - Jump to address NNN */
        case 0x1000: {
            op_1nnn(c);
            break;
        }
        /* 2nnn - Execute subroutine starting at address NNN */
        case 0x2000: {
            op_2nnn(c);
            break;
        }
        /* 3xnn - Skip the following instruction if the value of register VX equals NN */
        case 0x3000: {
            op_3xnn(c);
            break;
        }
        /* 4xnn - Skip the following instruction if the value of register VX is not equal to NN */
        case 0x4000: {
            op_4xnn(c);
            break;
        }
        /* 5xy0 - Skip the following instruction if the value of register VX is equal to the value of register VY */
        case 0x5000: {
            op_5xy0(c);
            break;
        }
        /* 6xnn - Store number NN in register VX */
        case 0x6000: {
            op_6xnn(c);
            break;
        }
        /* 7xnn - Add the value NN to register VX */
        case 0x7000: {
            op_7xnn(c);
            break;
        }
        case 0x8000: {
            unsigned char a = get_reg_value(c, get_opcode_x(c));
            unsigned char b = get_reg_value(c, get_opcode_y(c));

            /* 8xy0 - Store the value of register VY in register VX */
            if (test_opcode(c, 0x000F, 0)) {
                op_8xy0(c, a, b);
            }
            /* 8xy1 - Set VX to VX OR VY */
            else if (test_opcode(c, 0x000F, 1)) {
                op_8xy1(c, a, b);
            }
            /* 8xy2 - Set VX to VX AND VY */
            else if (test_opcode(c, 0x000F, 2)) {
                op_8xy2(c, a, b);
            }
            /* 8xy3 - Set VX to VX XOR VY */
            else if (test_opcode(c, 0x000F, 3)) {
                op_8xy3(c, a, b);
            }
            /* Add the value of register VY to register VX
             * Set VF to 01 if a carry occurs
             * Set VF to 00 if a carry does not occur
             */
            else if (test_opcode(c, 0x000F, 4)) {
                op_8xy4(c, a, b);
            }
            /* Subtract the value of register VY from register VX
             * Set VF to 00 if a borrow occurs
             * Set VF to 01 if a borrow does not occur
             */
            else if (test_opcode(c, 0x000F, 5)) {
                op_8xy5(c, a, b);
            }
            /* 8xy6 - Store the value of register VY shifted right one bit in register VX
             *        Set register VF to the least significant bit prior to the shift
             */
            else if (test_opcode(c, 0x000F, 6)) {
                op_8xy6(c, b);
            }
            /* 8xy7 - Set register VX to the value of VY minus VX
             *        Set VF to 00 if a borrow occurs
             *        Set VF to 01 if a borrow does not occur
             */
            else if (test_opcode(c, 0x000F, 7)) {
                op_8xy7(c, a, b);
            }
            /* 8xyE - Store the value of register VY shifted left one bit in register VX
             *        Set register VF to the most significant bit prior to the shift
             */
            else if (test_opcode(c, 0x000F, 0xE)) {
                op_8xyE(c, b);
            }

            break;
        }
        /* 9xy0 - Skip the following instruction if the value of register VX is not equal to the value of register VY */
        case 0x9000: {
            op_9xy0(c);
            break;
        }
        /* Annn - Store memory address NNN in register I */
        case 0xA000: {
            op_Annn(c);
            break;
        }
        /* Bnnn - Jump to address NNN + V0 */
        case 0xB000: {
            op_Bnnn(c);
            break;
        }
        /* Cxnn - Set VX to a random number with a mask of NN */
        case 0xC000: {
            op_Cxnn(c);
            break;
        }
        /* Dxyn - DRW Vx, Vy, nibble
         * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
         *
         * The interpreter reads n bytes from memory, starting at the address stored in I. These bytes
         * are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the
         * existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set
         * to 0. If the sprite is positioned so part of it is outside the coordinates of the display,
         * it wraps around to the opposite side of the screen.
         */
        case 0xD000: {
            op_Dxyn(c);
            break;
        }
        case 0xE000: {
            /* Ex9E - Skip the following instruction if the key corresponding to the hex value currently
             * stored in register VX is pressed */
            unsigned char a = get_reg_value(c, get_opcode_x(c));
            if (test_opcode(c, 0x00FF, 0x009E) && get_key_value(c, a)) {
                op_Ex9E(c);
            }
            /* ExA1 - Skip the following instruction if the key corresponding to the hex value currently
             * stored in register VX is not pressed */
            else if (test_opcode(c, 0x00FF, 0x00A1) && !get_key_value(c, a)) {
                op_ExA1(c);
            }

            break;
        }
        case 0xF000: {
            /* Fx07 - Store the current value of the delay timer in register VX */
            if (test_opcode(c, 0x000F, 0x0007)) {
                op_Fx07(c);
            }
            /* Fx0A - Wait for a keypress and store the result in register VX */
            else if (test_opcode(c, 0x000F, 0x000A)) {
                op_Fx0A(c);
            }
            /* Fx15 - Set the delay timer to the value of register VX */
            else if (test_opcode(c, 0x00FF, 0x0015)) {
                op_Fx15(c);
            }
            /* Fx18 - Set the sound timer to the value of register VX */
            else if (test_opcode(c, 0x00FF, 0x0018)) {
                op_Fx18(c);
            }
            /* Fx1E - Add the value stored in register VX to register I */
            else if (test_opcode(c, 0x00FF, 0x001E)) {
                op_Fx1E(c);
            }
            /* Fx29 - Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX */
            else if (test_opcode(c, 0x00FF, 0x0029)) {
                /* fontset starts at 0x50, translate that position by the value of Vx
                 * multiplied by the sprite width (5 bytes)
                 */
                op_Fx29(c);
            }
            /* Fx33 - Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2 */
            else if (test_opcode(c, 0x00FF, 0x0033)) {
                op_Fx33(c);
            }
            /* Fx55 - Store the values of registers V0 to VX inclusive in memory starting at address I
             *        I is set to I + X + 1 after operation */
            else if (test_opcode(c, 0x00FF, 0x0055)) {
                op_Fx55(c);
            }
            /* Fx65 - Fill registers V0 to VX inclusive with the values stored in memory starting at address I
             *        I is set to I + X + 1 after operation */
            else if (test_opcode(c, 0x00FF, 0x0065)) {
                op_Fx65(c);
            }

            break;
        }
        default: {
            fprintf(stderr, "unrecognized opcode %x\n", c->opcode);
        }
    }

    if (!c->pause) {
        pc_increment(c);

        // update timers
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
}

/* TODO: fix this shit */
void load_file(chip8 *c, const char *s)
{
    FILE *fp;

    // check for valid file pointer
    if ((fp = fopen(s, "r")) == NULL) {
        fprintf(stderr, "unable to open %s for reading\n", s);
        exit(1);
    }

    // temporary memory value, file char input
    unsigned char temp = 0;
    unsigned char hex  = 0;

    // base exponent
    int  counter = 1;
    set_pc(c, 0x200);

    // iterate over each character
    while (hex != EOF) {
        hex = fgetc(fp);

        if (hex == EOF)
            break;

        printf("%x\n", hex);
        // if letter or digit
        if ((hex >= '0' && hex <= '9') || (tolower(hex) >= 'a' && tolower(hex) <= 'f')) {
            // convert every byte from ASCII to numerical values
            if (tolower(hex) >= 'a' && tolower(hex) <= 'f') {
                temp += (tolower(hex) - 'a' + 10) * pow(16.f, counter);
            } else {
                temp += ((hex - '0') * pow(16.f, counter));
            }
            // MSB -> LSB (left -> right)
            counter--;

            // have iterated over at least one byte
            if (counter == -1) {
                // printf("%x\n", temp);
                // reset base exponent
                counter = 1;
                // store the program data in memory
                c->memory[get_pc(c)] = temp;
                // increment the PC by one b/c we're going one byte at a time
                set_pc(c, get_pc(c) + 1);
                // reset the temp memory value
                temp = 0;
            }
        }
    }

    set_pc(c, 0x200);
    fclose(fp);
}

/* Getters */
unsigned char  get_reg_value(chip8 *c, unsigned int i)
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

unsigned short get_sp(chip8 *c)
{
    return c->SP;
}

unsigned short get_stack_top(chip8 *c)
{
    return c->stack[c->SP];
}

unsigned char  get_display_value(chip8 *c, unsigned int x, unsigned int y)
{
    return c->display[x + y * W_WIDTH];
}

unsigned char *get_keys(chip8 *c)
{
    return c->keys;
}

unsigned char  get_key_value(chip8 *c, unsigned int i)
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

unsigned char  get_opcode_nn(chip8 *c)
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

bool test_opcode(chip8 *c, unsigned int bitmask, unsigned int value)
{
    return (c->opcode & bitmask) == value;
}

/* Setters */
void set_reg_value(chip8 *c, unsigned int i, unsigned char n)
{
    c->V[i] = n;
}

void set_addr_value(chip8 *c, unsigned char n)
{
    c->memory[get_addr(c)] = n;
}

void set_addr(chip8 *c, unsigned short i)
{
    c->I = i;
}

void set_pc(chip8 *c, unsigned short n)
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

void stack_push(chip8 *c, unsigned short n)
{
    sp_increment(c);
    c->stack[get_sp(c)] = n;
}

void set_key_value(chip8 *c, unsigned int i, unsigned char n)
{
    c->keys[i] = n;
}

void set_dt(chip8 *c, unsigned char n)
{
    c->DT = n;
}

void set_st(chip8 *c, unsigned char n)
{
    c->ST = n;
}

bool set_display_value(chip8 *c, unsigned int x, unsigned int y, unsigned char n)
{
    unsigned char temp = get_display_value(c, x, y);
    c->display[x + (y * W_WIDTH)] ^= n;

    // pixel got erased
    if ((temp == 1 && get_display_value(c, x, y) == 0)) {
        return 1;
    }

    return 0;
}

/* Instructions */
void  op_00E0(chip8 *c)
{
    clear_display(c);
}

void  op_00EE(chip8 *c)
{
    set_pc(c, get_stack_top(c));
    stack_pop(c);
}

void  op_1nnn(chip8 *c)
{
    set_pc(c, get_opcode_nnn(c));
}

void  op_2nnn(chip8 *c)
{
    stack_push(c, get_pc(c));
    set_pc(c, get_opcode_nnn(c));
}

void  op_3xnn(chip8 *c)
{
    if (get_reg_value(c, get_opcode_x(c)) == get_opcode_nn(c)) {
        pc_increment(c);
    }
}

void  op_4xnn(chip8 *c)
{
    if (get_reg_value(c, get_opcode_x(c)) != get_opcode_nn(c)) {
        pc_increment(c);
    }
}

void  op_5xy0(chip8 *c)
{
    if (get_reg_value(c, get_opcode_x(c)) == get_reg_value(c, get_opcode_y(c))) {
        pc_increment(c);
    }
}

void  op_6xnn(chip8 *c)
{
    set_reg_value(c, get_opcode_x(c), get_opcode_nn(c));
}

void  op_7xnn(chip8 *c)
{
    set_reg_value(c, get_opcode_x(c), get_opcode_nn(c));
}

void  op_8xy0(chip8 *c, unsigned char a, unsigned char b)
{
    set_reg_value(c, get_opcode_x(c), b);
}

void  op_8xy1(chip8 *c, unsigned char a, unsigned char b)
{
    set_reg_value(c, get_opcode_x(c), a | b);
}

void  op_8xy2(chip8 *c, unsigned char a, unsigned char b)
{
    set_reg_value(c, get_opcode_x(c), a & b);
}

void  op_8xy3(chip8 *c, unsigned char a, unsigned char b)
{
    set_reg_value(c, get_opcode_x(c), a ^ b);
}

void  op_8xy4(chip8 *c, unsigned char a, unsigned char b)
{
    if (a + b >= 256) {
        set_reg_value(c, 0xF, 1);
    } else {
        set_reg_value(c, 0xF, 0);
    }
    set_reg_value(c, get_opcode_x(c), (a + b) % 256);
}

void  op_8xy5(chip8 *c, unsigned char a, unsigned char b)
{
    // check the carry flag
    if (a < b) {
        set_reg_value(c, 0xF, 1);
    } else {
        set_reg_value(c, 0xF, 0);
    }
    set_reg_value(c, get_opcode_x(c), a - b); // set Vx = Vx - Vy
}

void  op_8xy6(chip8 *c, unsigned char n)
{
    if (n & 0x1 == 1) {
        set_reg_value(c, 0xF, 1);
    } else {
        set_reg_value(c, 0xF, 0);
    }

    set_reg_value(c, get_opcode_x(c), n >> 1);
}

void  op_8xy7(chip8 *c, unsigned char a, unsigned char b)
{
    if (b < a) {
        set_reg_value(c, 0xF, 1);
    } else {
        set_reg_value(c, 0xF, 0);
    }

    set_reg_value(c, get_opcode_x(c), b - a);
}

void  op_8xyE(chip8 *c, unsigned char n)
{
    if (n & 0x80 == 1) {
        set_reg_value(c, 0x000F, 1);
    } else {
        set_reg_value(c, 0x000F, 0);
    }

    set_reg_value(c, get_opcode_x(c), n << 1);
}

void  op_9xy0(chip8 *c)
{
    if (get_reg_value(c, get_opcode_x(c)) != get_reg_value(c, get_opcode_y(c))) {
        pc_increment(c);
    }
}

void  op_Annn(chip8 *c)
{
    set_addr(c, get_opcode_nnn(c));
}

void  op_Bnnn(chip8 *c)
{
    set_pc(c, get_opcode_nnn(c) + get_reg_value(c, 0));
}

void  op_Cxnn(chip8 *c)
{
    unsigned char n = rand() % 256;
    set_reg_value(c, get_opcode_x(c), n & get_opcode_nn(c));
}

void  op_Dxyn(chip8 *c)
{
    unsigned char n = c->opcode & 0xF;
    unsigned char x = get_reg_value(c, get_opcode_x(c));
    unsigned char y = get_reg_value(c, get_opcode_y(c));

    unsigned char pos_x[8] = {
         x,
        (x + 1) % (W_WIDTH * W_HEIGHT),
        (x + 2) % (W_WIDTH * W_HEIGHT),
        (x + 3) % (W_WIDTH * W_HEIGHT),
        (x + 4) % (W_WIDTH * W_HEIGHT),
        (x + 5) % (W_WIDTH * W_HEIGHT),
        (x + 6) % (W_WIDTH * W_HEIGHT),
        (x + 7) % (W_WIDTH * W_HEIGHT)
    };

    // iterate over n-bytes of the sprite in memory
    for (int i = get_addr(c); i < get_addr(c) + n; i++) {
        // char sprite_data = ;
        unsigned char sprite_data = c->memory[i];
        bool collision = 0;

        // the width of a fontset sprite is always 8 bits in Chip-8
        collision |= set_display_value(c, pos_x[0], y, (sprite_data & 0x80) >> 7); 
        collision |= set_display_value(c, pos_x[1], y, (sprite_data & 0x40) >> 6);
        collision |= set_display_value(c, pos_x[2], y, (sprite_data & 0x20) >> 5);
        collision |= set_display_value(c, pos_x[3], y, (sprite_data & 0x10) >> 4);
        collision |= set_display_value(c, pos_x[4], y, (sprite_data & 0x08) >> 3);
        collision |= set_display_value(c, pos_x[5], y, (sprite_data & 0x04) >> 2);
        collision |= set_display_value(c, pos_x[6], y, (sprite_data & 0x02) >> 1);
        collision |= set_display_value(c, pos_x[7], y, (sprite_data & 0x01));

        set_reg_value(c, 0xF, collision);

        y = (y + 1) % (W_WIDTH * W_HEIGHT);
    }
}

void  op_Ex9E(chip8 *c)
{
    pc_increment(c);
}

void  op_ExA1(chip8 *c)
{
    pc_increment(c);
}

void  op_Fx07(chip8 *c)
{
    set_reg_value(c, get_opcode_x(c), get_dt(c));
}

void  op_Fx0A(chip8 *c)
{
    c->pause = 1;

    if (c->pause && c->key_flag > 0) {
        set_reg_value(c, get_opcode_x(c), c->key_flag);
        c->pause    =  0;
        c->key_flag = -1;
    }
}

void  op_Fx15(chip8 *c)
{
    set_dt(c, get_reg_value(c, get_opcode_x(c)));
}

void  op_Fx18(chip8 *c)
{
    set_st(c, get_reg_value(c, get_opcode_x(c)));
}

void  op_Fx1E(chip8 *c)
{
    unsigned short sum = get_reg_value(c, get_opcode_x(c)) + get_addr(c);

    set_addr(c, sum);
}

void  op_Fx29(chip8 *c)
{
    set_addr(c, 0x50 + get_reg_value(c, get_opcode_x(c)) * 5);
}

void  op_Fx33(chip8 *c)
{
    unsigned char value = get_reg_value(c, get_opcode_x(c));

    c->memory[get_addr(c)]     = (value / 100) % 10;
    c->memory[get_addr(c) + 1] = (value / 10) % 10;
    c->memory[get_addr(c) + 2] = (value) % 10;
}

void  op_Fx55(chip8 *c)
{
    for (int i = 0; i <= get_opcode_x(c); i++) {
        c->memory[get_addr(c) + i] = get_reg_value(c, i);
    }
    set_addr(c, get_addr(c) + get_opcode_x(c) + 1);
}

void  op_Fx65(chip8 *c)
{
    for (int i = 0; i <= get_opcode_x(c); i++) {
        set_reg_value(c, i, c->memory[get_addr(c) + i]);
    }
    set_addr(c, get_addr(c) + get_opcode_x(c) + 1);
}
