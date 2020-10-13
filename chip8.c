#include "chip8.h"

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

	clear_display(c);
	// Clear stack
	// Clear registers V0-VF
	for (int i = 0; i < 16; i++) {
		c->stack[i] = 0;
		c->V[i] = 0;
	}
	// Clear memory
	for (int i = 0; i < 4096; i++) {
		c->memory = 0;
	}

	// Fontset data is stored at 0x50
	for (int i = 0; i < 80; i++) {
		memory[i + 0x50] = c->fontset[i];
	}

	// Reset timers
	c->DT = 0;
	c->ST = 0;
}

void execute(chip8 *c)
{
	// Merge two bytes to form a full instruction
	c->opcode = c->memory[c->PC] << 8 | memory[c->PC + 1];

	switch (c->opcode & 0xF000) {
		// 00E0 - CLS or 00EE - RET
		case (0x0000):
			if (c->opcode & 0x00E0 == 0x00E0) {
				// Clear the display
				clear_display(c);
				c->PC += 2;
			} else {
				// RET
				c->PC = c->stack[c->SP];
				c->SP--;
			}
			break;
		// 1nnn - JP addr
		case (0x1000):
			c->PC = c->opcode & 0x0FFF;
			break;
		// 2nnn - CALL addr
		case (0x2000):
			c->SP++;

			if (c->SP > 15) {
				fprintf(stderr, "Stack overflow\n");
			}

			c->stack[c->SP] = c->PC;
			c->PC += 2;
			break;
		// 3xkk - SE Vx, byte
		case (0x3000):
			if (c->V[c->opcode & 0x0F00 >> 8] == c->opcode & 0x00FF) {
				c->PC += 2;
			}
			c->PC += 2;
			break;
		// 4xkk - SNE Vx, byte
		case (0x4000):
			if (c->V[c->opcode & 0x0F00 >> 8] != c->opcode & 0x00FF) {
				c->PC += 2;
			}
			c->PC += 2;
			break;
		// 5xy0 - SE Vx, Vy
		case (0x5000):
			if (c->V[c->opcode & 0x0F00 >> 8] == c->V[c->opcode & 0x00F0 >> 4]) {
				c->PC += 2;
			}
			c->PC += 2;
			break;
		// 6xkk - LD Vx, byte
		case (0x6000):
			c->V[c->opcode & 0x0F00 >> 8] = c->opcode & 0x00FF;
			c->PC += 2;
			break;
		// 7xkk - ADD Vx, byte
		case (0x7000):
			c->V[c->opcode & 0x0F00 >> 8] += c->opcode & 0x00FF;
			c->PC += 2;
			break;
		case (0x8000):
			// 8xy0 - LD Vx, Vy
			if (c->opcode & 0x800F == 0x8000) {
				c->V[c->opcode & 0x0F00 >> 8] = c->V[c->opcode & 0x00F0 >> 4];
			// 8xy1 - OR Vx, Vy
			} else if (c->opcode & 0x800F == 0x8001) {
				c->V[c->opcode & 0x0F00 >> 8] |= c->V[c->opcode & 0x00F0 >> 4];
			}
			c->PC += 2;
		// Annn - LD I, addr
		case (0xA000):
			c->I = c->opcode & 0x0FFF;
			c->PC += 2;
			break;
		// Bnnn - JP V0, addr
		case (0xB000):
			c->PC += c->opcode & 0x0FFF;
			break;
		// Cxkk - RND Vx, byte
		case (0xC000):
			c->V[c->opcode & 0x0F00 >> 8] = (rand() % 256) & c->opcode & 0x00FF;
			c->PC += 2;
			break;
		default:
			fprintf(stderr, "Invalid opcode %i at %i\n", c->opcode, c->PC);
			exit(1);
			break;
	}

	// Update timers	
}

