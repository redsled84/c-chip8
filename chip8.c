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
	
	for (int i = 0; i < 16; i++) {
		c->stack[i] = 0;  // Clear stack
		c->V[i] = 0;      // Clear registers V0-VF
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
		// 9xy0 - SNE Vx, Vy
		case (0x9000):
			if (c->V[c->opcode & 0x0F00 >> 8] != c->V[c->opcode & 0x00F0 >> 4]) {
				c->PC += 2;
			}
			c->PC += 2;
			break;
		// Annn - LD I, addr
		case (0xA000):
			c->I = c->opcode & 0x0FFF;
			c->PC += 2;
			break;
		// Bnnn - JP V0, addr
		case (0xB000):
			c->PC = c->V[0] + c->opcode & 0x0FFF;
			break;
		// Cxkk - RND Vx, byte
		case (0xC000):
			c->V[c->opcode & 0x0F00 >> 8] = (rand() % 256) & c->opcode & 0x00FF;
			c->PC += 2;
			break;
		// Dxyn - DRW Vx, Vy, nibble
		case (0xD000):
			break;
		default:
			fprintf(stderr, "Invalid opcode %i at %i\n", c->opcode, c->PC);
			exit(1);
			break;
	}

	// Update timers
	if (c->DT > 0) {

	}

	if (c->ST > 0) {
		
	}
}

