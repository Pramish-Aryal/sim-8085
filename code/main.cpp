#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define panic(str) assert(0);

enum Registers {
	REG_A = 0,
	REG_F,
	REG_B,
	REG_C,
	REG_D,
	REG_E,
	REG_L,
	REG_H,
	REG_COUNT,
	REG_M,
};

enum Flags: uint8_t {
	FLAG_NONE = 0,
	FLAG_S = 1 << 7,
	FLAG_Z = 1 << 6,
	FLAG_AC = 1 << 4,
	FLAG_P = 1 << 3,
	FLAG_CY = 1,
};

static uint8_t g_memory[64 * 1024];
static uint8_t registers[REG_COUNT] = {};
static uint8_t& flags = registers[REG_F];

enum Instruction_Set {
	MOV_A_A = 0x7F,
	MOV_A_B = 0x78,
	MOV_A_C = 0x79,
	MOV_A_D = 0x7A,
	MOV_A_E = 0x7B,
	MOV_A_H = 0x7C,
	MOV_A_L = 0x7D,
	MOV_A_M = 0x7E,

	MOV_B_A = 0x47,
	MOV_B_B = 0x40,
	MOV_B_C = 0x41,
	MOV_B_D = 0x42,
	MOV_B_E = 0x43,
	MOV_B_H = 0x44,
	MOV_B_L = 0x45,
	MOV_B_M = 0x46,

	MOV_C_A = 0x4F,
	MOV_C_B = 0x48,
	MOV_C_C = 0x49,
	MOV_C_D = 0x4A,
	MOV_C_E = 0x4B,
	MOV_C_H = 0x4C,
	MOV_C_L = 0x4D,
	MOV_C_M = 0x4E,

	MOV_D_A = 0x57,
	MOV_D_B = 0x50,
	MOV_D_C = 0x51,
	MOV_D_D = 0x52,
	MOV_D_E = 0x53,
	MOV_D_H = 0x54,
	MOV_D_L = 0x55,
	MOV_D_M = 0x56,

	MOV_E_A = 0x5F,
	MOV_E_B = 0x58,
	MOV_E_C = 0x59,
	MOV_E_D = 0x5A,
	MOV_E_E = 0x5B,
	MOV_E_H = 0x5C,
	MOV_E_L = 0x5D,
	MOV_E_M = 0x5E,



	MOV_H_A = 0x67,
	MOV_H_B = 0x60,
	MOV_H_C = 0x61,
	MOV_H_D = 0x62,
	MOV_H_E = 0x63,
	MOV_H_H = 0x64,
	MOV_H_L = 0x65,
	MOV_H_M = 0x66,

	MOV_L_A = 0x6F,
	MOV_L_B = 0x68,
	MOV_L_C = 0x69,
	MOV_L_D = 0x6A,
	MOV_L_E = 0x6B,
	MOV_L_H = 0x6C,
	MOV_L_L = 0x6D,
	MOV_L_M = 0x6E,

	MOV_M_A = 0x77,
	MOV_M_B = 0x70,
	MOV_M_C = 0x71,
	MOV_M_D = 0x72,
	MOV_M_E = 0x73,
	MOV_M_H = 0x74,
	MOV_M_L = 0x75,

	MVI_A = 0x3E,
	MVI_B = 0x06,
	MVI_C = 0x0E,
	MVI_D = 0x16,
	MVI_E = 0x1E,
	MVI_H = 0x26,
	MVI_L = 0x2E,
	MVI_M = 0x36,

	LXI_B = 0x01,
	LXI_D = 0x11,
	LXI_H = 0x21,
	LXI_SP = 0x31,

	ADD_A = 0x87,
	ADD_B = 0x80,
	ADD_C = 0x81,
	ADD_D = 0x82,
	ADD_E = 0x83,
	ADD_H = 0x84,
	ADD_L = 0x85,
	ADD_M = 0x86,

	ADC_A = 0x8F,
	ADC_B = 0x88,
	ADC_C = 0x89,
	ADC_D = 0x8A,
	ADC_E = 0x8B,
	ADC_H = 0x8C,
	ADC_L = 0x8D,
	ADC_M = 0x8E,

	SUB_A = 0x97,
	SUB_B = 0x90,
	SUB_C = 0x91,
	SUB_D = 0x92,
	SUB_E = 0x93,
	SUB_H = 0x94,
	SUB_L = 0x95,
	SUB_M = 0x96,

	SBB_A = 0x9F,
	SBB_B = 0x98,
	SBB_C = 0x99,
	SBB_D = 0x9A,
	SBB_E = 0x9B,
	SBB_H = 0x9C,
	SBB_L = 0x9D,
	SBB_M = 0x9E,

	INR_A = 0x3C,
	INR_B = 0x04,
	INR_C = 0x0C,
	INR_D = 0x14,
	INR_E = 0x1C,
	INR_H = 0x24,
	INR_L = 0x2C,
	INR_M = 0x34,

	INX_B = 0x03,
	INX_D = 0x13,
	INX_H = 0x23,
	INX_SP = 0x33,

	DCR_A = 0x3C + 1,
	DCR_B = 0x04 + 1,
	DCR_C = 0x0C + 1,
	DCR_D = 0x14 + 1,
	DCR_E = 0x1C + 1,
	DCR_H = 0x24 + 1,
	DCR_L = 0x2C + 1,
	DCR_M = 0x34 + 1,

	DCX_B = 0x0B,
	DCX_D = 0x1B,
	DCX_H = 0x2B,
	DCX_SP = 0x3B,

	JMP = 0xC3,
	JNZ = 0xC2,
	JZ = 0xCA,
	JNC = 0xD2,
	JC = 0xDA,
	JPO = 0xE2,
	JPE = 0xEA,
	JP = 0xF2,
	JM = 0xFA,
	PCHL = 0xE9,

	NOP = 0x00,
	HLT = 0x76,

	CMP_M = 0xBE,
	CPI = 0xFE,



};
/*

	2000H		START:	LXI H, 2040H	Load size of array
	2003H	 	MVI D, 00H	Clear D registers to set up a flag
	2005H	 	MOV C, M	Set C registers with number of elements in list
	2006H	 	DCR C	Decrement C
	2007H	 	INX H	Increment memory to access list
	2008H		CHECK	MOV A, M	Retrieve list element in Accumulator
	2009H	 	INX H	Increment memory to access next element
	200AH	 	CMP M	Compare Accumulator with next element
	200BH	 	JC NEXTBYTE	If accumulator is less then jump to NEXTBYTE
	200EH	 	JZ NEXTBYTE	If accumulator is equal then jump to NEXTBYTE
	2011H	 	MOV B, M	Swap the two elements
	2012H	 	MOV M, A
	2013H	 	DCX H
	2014H	 	MOV M, B
	2015H	 	INX H
	2016H	 	MVI D, 01H	If exchange occurs save 01 in D registers
	2018H		NEXTBYTE	DCR C	Decrement C for next iteration
	2019H	 	JNZ CHECK	Jump to CHECK if C>0
	201CH	 	MOV A, D	Transfer contents of D to Accumulator
	201DH	 	CPI 01H	Compare accumulator contents with 01H
	201FH	 	JZ START	Jump to START if D=01H
	2022H	 	HLT	HALT
	*
	*

START:	LXI H, 2040H	;Load size of array
MVI D, 00H	;Clear D registers to set up a flag
MOV C, M	;Set C registers with number of elements in list
DCR C	;Decrement C
INX H	;Increment memory to access list
CHECK:	MOV A, M	;Retrieve list element in Accumulator
INX H	;Increment memory to access next element
CMP M	;Compare Accumulator with next element
JC NEXTBYTE	;If accumulator is less then jump to NEXTBYTE
JZ NEXTBYTE	;If accumulator is equal then jump to NEXTBYTE
MOV B, M	;Swap the two elements
MOV M, A
DCX H
MOV M, B
INX H
MVI D, 01H	;If exchange occurs save 01 in D registers
NEXTBYTE:	DCR C	;Decrement C for next iteration
JNZ CHECK	;Jump to CHECK if C>0
MOV A, D	;Transfer contents of D to Accumulator
CPI 01H	;Compare accumulator contents with 01H
JZ START	;Jump to START if D=01H
HLT	;HALT
*
*
*
*
*/

#define BC_PAIR ((uint16_t)registers[REG_B] << 8 | (uint16_t)registers[REG_C])
#define DE_PAIR ((uint16_t)registers[REG_D] << 8 | (uint16_t)registers[REG_E])
#define HL_PAIR ((uint16_t)registers[REG_H] << 8 | (uint16_t)registers[REG_L])

void mov(int dst, int src) {
    if (src == dst) return;
    uint8_t src_val;
    src_val = (src == REG_M) ? g_memory[HL_PAIR] : registers[src];
    (dst == REG_M) ? g_memory[HL_PAIR] = src_val : registers[dst] = src_val;
}

int main()
{
	uint8_t* ip = g_memory + 0x2000;

	//START:	LXI H, 2040H
	*ip++ = LXI_H;
	*ip++ = 0x40;
	*ip++ = 0x20;
	//MVI D, 00H
	*ip++ = MVI_D;
	*ip++ = 0x00;
	//MOV C, M
	*ip++ = MOV_C_M;
	//DCR C
	*ip++ = DCR_C;
	//INX H
	*ip++ = INX_H;
	//CHECK:	MOV A, M
	*ip++ = MOV_A_M;
	//INX H
	*ip++ = INX_H;
	//CMP M
	*ip++ = CMP_M;
	//JC NEXTBYTE
	*ip++ = JC;
	*ip++ = 0x18;
	*ip++ = 0x20;
	//JZ NEXTBYTE
	*ip++ = JZ;
	*ip++ = 0x18;
	*ip++ = 0x20;
	//MOV B, M
	*ip++ = MOV_B_M;
	//MOV M, A
	*ip++ = MOV_M_A;
	//DCX H
	*ip++ = DCX_H;
	//MOV M, B
	*ip++ = MOV_M_B;
	//INX H
	*ip++ = INX_H;
	//MVI D, 01H
	*ip++ = MVI_D;
	*ip++ = 0x01;
	//NEXTBYTE:	DCR C
	*ip++ = DCR_C;
	//JNZ CHECK
	*ip++ = JNZ;
	*ip++ = 0x08;
	*ip++ = 0x20;
	//MOV A, D
	*ip++ = MOV_A_D;
	//CPI 01H
	*ip++ = CPI;
	*ip++ = 0x01;
	//JZ START
	*ip++ = JZ;
	*ip++ = 0x00;
	*ip++ = 0x20;
	//HLT
	*ip++ = HLT;

	ip = g_memory + 0x2000;
	uint16_t PC = 0x2000;

	g_memory[0x2040] = 5;
	uint8_t numbers[] = { 9, 3, 2, 4, 1 };
	memcpy(g_memory + 0x2041, numbers, sizeof(numbers));
	int iter = 0;
	for (bool running = true; running; iter++) {
		switch (g_memory[PC]) {
			case LXI_H:
				(* ((uint16_t*)&registers[REG_L])) = ((uint16_t*)(g_memory + ++PC))[0];
				PC += 2;
				break;

			case MVI_D:
				registers[REG_D] = g_memory[++PC];
				++PC;
				break;

			case MOV_A_A:
				++PC;
				break;
			case MOV_A_B:
				mov(REG_A, REG_B);
				++PC;
				break;
			case MOV_A_C:
				mov(REG_A, REG_C);
				++PC;
				break;
			case MOV_A_D:
				mov(REG_A, REG_D);
				++PC;
				break;
			case MOV_A_E:
				mov(REG_A, REG_E);
				++PC;
				break;
			case MOV_A_H:
				mov(REG_A, REG_H);
				++PC;
				break;
			case MOV_A_L:
				mov(REG_A, REG_L);
				++PC;
				break;
			case MOV_A_M:
				mov(REG_A, REG_M);
				++PC;
				break;

			case MOV_B_A:
				mov(REG_B, REG_A);
				++PC;
				break;
			case MOV_B_B:
				++PC;
				break;
			case MOV_B_C:
				mov(REG_B, REG_C);
				++PC;
				break;
			case MOV_B_D:
				mov(REG_B, REG_D);
				++PC;
				break;
			case MOV_B_E:
				mov(REG_B, REG_E);
				++PC;
				break;
			case MOV_B_H:
				mov(REG_B, REG_H);
				++PC;
				break;
			case MOV_B_L:
				mov(REG_B, REG_L);
				++PC;
				break;
			case MOV_B_M:
				mov(REG_B, REG_M);
				++PC;
				break;

			case MOV_C_A:
				mov(REG_C, REG_A);
				++PC;
				break;
			case MOV_C_B:
				mov(REG_C, REG_B);
				++PC;
				break;
			case MOV_C_C:
				++PC;
				break;
			case MOV_C_D:
				mov(REG_C, REG_D);
				++PC;
				break;
			case MOV_C_E:
				mov(REG_C, REG_E);
				++PC;
				break;
			case MOV_C_H:
				mov(REG_C, REG_H);
				++PC;
				break;
			case MOV_C_L:
				mov(REG_C, REG_L);
				++PC;
				break;
			case MOV_C_M:
				mov(REG_C, REG_M);
				++PC;
				break;

			case MOV_D_A:
				mov(REG_D, REG_A);
				++PC;
				break;
			case MOV_D_B:
				mov(REG_D, REG_B);
				++PC;
				break;
			case MOV_D_C:
				mov(REG_D, REG_C);
				++PC;
				break;
			case MOV_D_D:
				++PC;
				break;
			case MOV_D_E:
				mov(REG_D, REG_E);
				++PC;
				break;
			case MOV_D_H:
				mov(REG_D, REG_H);
				++PC;
				break;
			case MOV_D_L:
				mov(REG_D, REG_L);
				++PC;
				break;
			case MOV_D_M:
				mov(REG_D, REG_M);
				++PC;
				break;

			case MOV_E_A:
				mov(REG_E, REG_A);
				++PC;
				break;
			case MOV_E_B:
				mov(REG_E, REG_B);
				++PC;
				break;
			case MOV_E_C:
				mov(REG_E, REG_C);
				++PC;
				break;
			case MOV_E_D:
				mov(REG_E, REG_D);
				++PC;
				break;
			case MOV_E_E:
				++PC;
				break;
			case MOV_E_H:
				mov(REG_E, REG_H);
				++PC;
				break;
			case MOV_E_L:
				mov(REG_E, REG_L);
				++PC;
				break;
			case MOV_E_M:
				mov(REG_E, REG_M);
				++PC;
				break;

			case MOV_H_A:
				mov(REG_H, REG_A);
				++PC;
				break;
			case MOV_H_B:
				mov(REG_H, REG_B);
				++PC;
				break;
			case MOV_H_C:
				mov(REG_H, REG_C);
				++PC;
				break;
			case MOV_H_D:
				mov(REG_H, REG_D);
				++PC;
				break;
			case MOV_H_E:
				mov(REG_H, REG_E);
				++PC;
				break;
			case MOV_H_H:
				++PC;
				break;
			case MOV_H_L:
				mov(REG_H, REG_L);
				++PC;
				break;
			case MOV_H_M:
				mov(REG_H, REG_M);
				++PC;
				break;

			case MOV_L_A:
				mov(REG_L, REG_A);
				++PC;
				break;
			case MOV_L_B:
				mov(REG_L, REG_B);
				++PC;
				break;
			case MOV_L_C:
				mov(REG_L, REG_C);
				++PC;
				break;
			case MOV_L_D:
				mov(REG_L, REG_D);
				++PC;
				break;
			case MOV_L_E:
				mov(REG_L, REG_E);
				++PC;
				break;
			case MOV_L_H:
				mov(REG_L, REG_H);
				++PC;
				break;
			case MOV_L_L:
				++PC;
				break;
			case MOV_L_M:
				mov(REG_L, REG_M);
				++PC;
				break;

			case MOV_M_A:
				mov(REG_M, REG_A);
				++PC;
				break;
			case MOV_M_B:
				mov(REG_M, REG_B);
				++PC;
				break;
			case MOV_M_C:
				mov(REG_M, REG_C);
				++PC;
				break;
			case MOV_M_D:
				mov(REG_M, REG_D);
				++PC;
				break;
			case MOV_M_E:
				mov(REG_M, REG_E);
				++PC;
				break;
			case MOV_M_H:
				mov(REG_M, REG_H);
				++PC;
				break;
			case MOV_M_L:
				mov(REG_M, REG_L);
				++PC;
				break;

			case DCR_C:
				registers[REG_C]--;
				flags |= FLAG_Z * (registers[REG_C] == 0);
				flags |= FLAG_S * (registers[REG_C] & (0x80));
				PC++;
				break;

			case DCX_H:
				if ((*((uint16_t*)&registers[REG_L]))-- == 0x2003)
					__debugbreak();
				PC++;
				break;

			case INX_H:
				//registers[REG_H] += (++registers[REG_L] == 0);
				if((*((uint16_t*)&registers[REG_L]))++ == 0x2003)
					__debugbreak();
				PC++;
				break;

			case CMP_M: {
				//A - B
				uint16_t reg_m = (uint16_t)registers[REG_H] << 8 | registers[REG_L];
				int result = (int8_t)registers[REG_A] - (int8_t)g_memory[reg_m];
				if (result > 0) {
					flags &= ~FLAG_CY;
					flags &= ~FLAG_Z;
					flags &= ~FLAG_S;
				} else if (result == 0) {
					flags &= ~FLAG_S;
					flags |= FLAG_Z;
				} else {
					flags |= FLAG_S | FLAG_CY;
					flags &= ~FLAG_Z;
				}
				PC++;
			} break;

			case CPI: {
				//A - B
				int result = (int8_t)registers[REG_A] - g_memory[++PC];
				if (result > 0) {
					flags &= ~FLAG_CY;
					flags &= ~FLAG_Z;
					flags &= ~FLAG_S;
				} else if (result == 0) {
					flags &= ~FLAG_S;
					flags |= FLAG_Z;
				} else {
					flags |= FLAG_S | FLAG_CY;
					flags &= ~FLAG_Z;
				}
				PC++;
			} break;

			case JC:
				if (flags & FLAG_CY)
					PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1];
				else
					PC += 3;
				break;

			case JZ:
				if (flags & FLAG_Z)
					PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1];
				else
					PC += 3;
				break;

			case JNZ:
				if (flags & FLAG_Z)
					PC += 3;
				else
					PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1];
				break;

			case HLT:
				running = false;
				++PC;
				break;

			case NOP:
				break;

			default: panic("Should be unreachable");
		}

	}

	memcpy(numbers, g_memory + 0x2041, sizeof(numbers));
	return 0;
}

