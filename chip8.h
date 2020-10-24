#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_MEMORY 4096
#define W_WIDTH      64
#define W_HEIGHT     32

/* Fontset data */
extern unsigned char fontset[];

typedef struct chip8_t {
    /* Initialize the memory (4096 bytes) */
    unsigned char memory[MAX_MEMORY];
    /* Variable for storing the current opcode (2 bytes) */
    unsigned short opcode;
    /* 16 8-bit general purpose registers, last register is the instruction flag */
    unsigned char  V[16];
    /* Register for indexing memory addresses */
    unsigned short I;
    /* Program counter (current instruction being executed) */
    unsigned short PC;
    /* Stack pointer */
    unsigned char SP;
    /* Stack */
    unsigned short stack[16];
    /* Hexadecimal keypad */
    unsigned char keys[16];
    /* Delay and sound timer */
    unsigned char DT, ST;
    /* Display */
    unsigned char display[W_WIDTH * W_HEIGHT];

    char pause;
} chip8;

/* Main operations */
void clear_display(chip8 *c);
void initialize(chip8 *c);
void execute_instruction(chip8 *c);
void load_file(chip8 *c, const char *s);

/* Getters */
char  get_register_value(chip8 *c, int i);
char  get_address_value(chip8 *c); // returns value at memory location I
short get_pc(chip8 *c);
short get_stack_top(chip8 *c);
char  get_display_value(chip8 *c, int x, int y);
char *get_keys(chip8 *c);
char  get_key_value(chip8 *c, int i);
short get_opcode(chip8 *c);
short get_opcode_nnn(chip8 *c);
char  get_opcode_x(chip8 *c);
char  get_opcode_y(chip8 *c);
char  get_opcode_kk(chip8 *c);
char  get_dt(chip8 *c);
char  get_st(chip8 *c);

/* Setters */
void set_register_value(chip8 *c, int i, char n);
void set_address_value(chip8 *c, char n);
void set_address(chip8 *c, short i);
void set_pc(chip8 *c, short n);
void pc_increment(chip8 *c);
void sp_increment(chip8 *c);
void stack_pop(chip8 *c);
void set_key_value(chip8 *c, int i, char n);
void set_dt(chip8 *c, char n);
void set_st(chip8 *c, char n);

#endif
