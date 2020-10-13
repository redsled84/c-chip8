#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>

/* CHIP-8 memory map
 * -----------------
 *
 * +---------------+= 0xFFF (4095) End of Chip-8 RAM
 * |               |
 * |               |
 * |               |
 * |               |
 * |               |
 * | 0x200 to 0xFFF|
 * |     Chip-8    |
 * | Program / Data|
 * |     Space     |
 * |               |
 * |               |
 * |               |
 * +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
 * |               |
 * |               |
 * |               |
 * +---------------+= 0x200 (512) Start of most Chip-8 programs
 * | 0x000 to 0x1FF|
 * | Reserved for  |
 * |  interpreter  |
 * +---------------+= 0x000 (0) Start of Chip-8 RAM
 */

#define MAX_MEMORY 4096
#define W_WIDTH      64
#define W_HEIGHT     32

typedef struct chip8_t {
	/* Initialize the memory (4096 bytes) */
	unsigned char memory[MAX_MEMORY] = {0};
	/* Variable for storing the current opcode */
	unsigned short opcode;
	/* 16 8-bit general purpose registers, last one is instruction flag */
	unsigned char  V[16];
	/* Register for storing memory addresses */
	unsigned short I;
	/* Program counter (currently executing instruction) */
	unsigned short PC;
	/* Stack pointer */
	unsigned char  SP;
	/* Stack */
	unsigned short stack[16];
	/* Hexadecimal keypad */
	unsigned char key[16];
	/* Delay and sound timer */
	unsigned char DT, ST;
	/* Display */
	unsigned char display[W_WIDTH * W_HEIGHT];

	/* Fontset data */
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

	bool draw_flag;
} chip8;

void clear_display(chip8 *c);
void initialize(chip8 *c);
void execute(chip8 *c);
void load_file(const char *s);
void set_keys(chip8 *c);

#endif
