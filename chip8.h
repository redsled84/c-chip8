#ifndef CHIP8_H
#define CHIP8_H

#include <math.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

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
    /* Program counter (current address being executed) */
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
    char key_flag;
} chip8;

/* Main operations */
void  clear_display        (chip8 *c);
void  initialize           (chip8 *c);
void  execute_instruction  (chip8 *c);
void  load_file            (chip8 *c, const char *s);

/* Getters */
unsigned char    get_reg_value     (chip8 *c, unsigned int i);
unsigned char    get_addr          (chip8 *c);
unsigned char    get_addr_value    (chip8 *c);
unsigned short   get_pc            (chip8 *c);
unsigned short   get_sp            (chip8 *c);
unsigned short   get_stack_top     (chip8 *c);
unsigned char    get_display_value (chip8 *c, unsigned int x, unsigned int y);
unsigned char   *get_keys          (chip8 *c);
unsigned char    get_key_value     (chip8 *c, unsigned int i);
unsigned short   get_opcode        (chip8 *c);
unsigned short   get_opcode_nnn    (chip8 *c);
unsigned char    get_opcode_x      (chip8 *c);
unsigned char    get_opcode_y      (chip8 *c);
unsigned char    get_opcode_nn     (chip8 *c);
unsigned char    get_dt            (chip8 *c);
unsigned char    get_st            (chip8 *c);
bool             test_opcode       (chip8 *c, unsigned int bitmask, unsigned int value);

/* Setters */
void  set_reg_value      (chip8 *c, unsigned int i, unsigned char n);
void  set_addr_value     (chip8 *c, unsigned char n);
void  set_addr           (chip8 *c, unsigned short i);
void  set_pc             (chip8 *c, unsigned short n);
void  pc_increment       (chip8 *c);
void  sp_increment       (chip8 *c);
void  sp_decrement       (chip8 *c);
void  stack_pop          (chip8 *c);
void  stack_push         (chip8 *c, unsigned short n);
void  set_key_value      (chip8 *c, unsigned int i, unsigned char n);
void  set_dt             (chip8 *c, unsigned char n);
void  set_st             (chip8 *c, unsigned char n);
bool  set_display_value  (chip8 *c, unsigned int x, unsigned int y, unsigned char n);

/* Instructions */
void  op_00E0(chip8 *c);
void  op_00EE(chip8 *c);
void  op_1nnn(chip8 *c);
void  op_2nnn(chip8 *c);
void  op_3xnn(chip8 *c);
void  op_4xnn(chip8 *c);
void  op_5xy0(chip8 *c);
void  op_6xnn(chip8 *c);
void  op_7xnn(chip8 *c);
void  op_8xy0(chip8 *c, unsigned char a, unsigned char b);
void  op_8xy1(chip8 *c, unsigned char a, unsigned char b);
void  op_8xy2(chip8 *c, unsigned char a, unsigned char b);
void  op_8xy3(chip8 *c, unsigned char a, unsigned char b);
void  op_8xy4(chip8 *c, unsigned char a, unsigned char b);
void  op_8xy5(chip8 *c, unsigned char a, unsigned char b);
void  op_8xy6(chip8 *c, unsigned char n);
void  op_8xy7(chip8 *c, unsigned char a, unsigned char b);
void  op_8xyE(chip8 *c, unsigned char n);
void  op_9xy0(chip8 *c);
void  op_Annn(chip8 *c);
void  op_Bnnn(chip8 *c);
void  op_Cxnn(chip8 *c);
void  op_Dxyn(chip8 *c);
void  op_Ex9E(chip8 *c);
void  op_ExA1(chip8 *c);
void  op_Fx07(chip8 *c);
void  op_Fx0A(chip8 *c);
void  op_Fx15(chip8 *c);
void  op_Fx18(chip8 *c);
void  op_Fx1E(chip8 *c);
void  op_Fx29(chip8 *c);
void  op_Fx33(chip8 *c);
void  op_Fx55(chip8 *c);
void  op_Fx65(chip8 *c);

#endif
