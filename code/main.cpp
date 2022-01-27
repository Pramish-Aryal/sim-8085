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
	FLAG_S  = 1 << 7,
	FLAG_Z  = 1 << 6,
	FLAG_AC = 1 << 4,
	FLAG_P  = 1 << 3,
	FLAG_CY = 1,
};

static uint8_t g_memory[64 * 1024];
static uint8_t registers[REG_COUNT] = {};
static uint8_t& flags = registers[REG_F];

enum Instruction_Set {
	/*======  Data Transfer Group ======*/
	MOV_A_A = 0x7F, MOV_B_A = 0x47, MOV_C_A = 0x4F, MOV_D_A = 0x57, MOV_E_A = 0x5F,
	MOV_A_B = 0x78, MOV_B_B = 0x40, MOV_C_B = 0x48, MOV_D_B = 0x50, MOV_E_B = 0x58,
	MOV_A_C = 0x79, MOV_B_C = 0x41, MOV_C_C = 0x49, MOV_D_C = 0x51, MOV_E_C = 0x59,
	MOV_A_D = 0x7A, MOV_B_D = 0x42, MOV_C_D = 0x4A, MOV_D_D = 0x52, MOV_E_D = 0x5A,
	MOV_A_E = 0x7B, MOV_B_E = 0x43, MOV_C_E = 0x4B, MOV_D_E = 0x53, MOV_E_E = 0x5B,
	MOV_A_H = 0x7C, MOV_B_H = 0x44, MOV_C_H = 0x4C, MOV_D_H = 0x54, MOV_E_H = 0x5C,
	MOV_A_L = 0x7D, MOV_B_L = 0x45, MOV_C_L = 0x4D, MOV_D_L = 0x55, MOV_E_L = 0x5D,
	MOV_A_M = 0x7E, MOV_B_M = 0x46, MOV_C_M = 0x4E, MOV_D_M = 0x56, MOV_E_M = 0x5E,

	MOV_H_A = 0x67, MOV_L_A = 0x6F, MOV_M_A = 0x77, MVI_A = 0x3E,
	MOV_H_B = 0x60, MOV_L_B = 0x68, MOV_M_B = 0x70, MVI_B = 0x06,
	MOV_H_C = 0x61, MOV_L_C = 0x69, MOV_M_C = 0x71, MVI_C = 0x0E,
	MOV_H_D = 0x62, MOV_L_D = 0x6A, MOV_M_D = 0x72, MVI_D = 0x16,
	MOV_H_E = 0x63, MOV_L_E = 0x6B, MOV_M_E = 0x73, MVI_E = 0x1E,
	MOV_H_H = 0x64, MOV_L_H = 0x6C, MOV_M_H = 0x74, MVI_H = 0x26,
	MOV_H_L = 0x65, MOV_L_L = 0x6D, MOV_M_L = 0x75, MVI_L = 0x2E,
	MOV_H_M = 0x66, MOV_L_M = 0x6E, XCHG    = 0xEB, MVI_M = 0x36,

	LXI_B  = 0x01, LDAX_B = 0x0A, STAX_B = 0x02,
	LXI_D  = 0x11, LDAX_D = 0x1A, STAX_D = 0x12,
	LXI_H  = 0x21, LHLD   = 0x2A, SHLD   = 0x22,
	LXI_SP = 0x31, LDA    = 0x3A, STA    = 0x32,

	/*======  Arithmetic and Logic Group ======*/
	ADD_A = 0x87, ADC_A = 0x8F, SUB_A = 0x97, SBB_A = 0x9F, INR_A = 0x3C, DCR_A = 0x3D,
	ADD_B = 0x80, ADC_B = 0x88, SUB_B = 0x90, SBB_B = 0x98, INR_B = 0x04, DCR_B = 0x05,
	ADD_C = 0x81, ADC_C = 0x89, SUB_C = 0x91, SBB_C = 0x99, INR_C = 0x0C, DCR_C = 0x0D,
	ADD_D = 0x82, ADC_D = 0x8A, SUB_D = 0x92, SBB_D = 0x9A, INR_D = 0x14, DCR_D = 0x15,
	ADD_E = 0x83, ADC_E = 0x8B, SUB_E = 0x93, SBB_E = 0x9B, INR_E = 0x1C, DCR_E = 0x1D,
	ADD_H = 0x84, ADC_H = 0x8C, SUB_H = 0x94, SBB_H = 0x9C, INR_H = 0x24, DCR_H = 0x25,
	ADD_L = 0x85, ADC_L = 0x8D, SUB_L = 0x95, SBB_L = 0x9D, INR_L = 0x2C, DCR_L = 0x2D,
	ADD_M = 0x86, ADC_M = 0x8E, SUB_M = 0x96, SBB_M = 0x9E, INR_M = 0x34, DCR_M = 0x35,

	INX_B  = 0x03, DCX_B  = 0x0B, DAD_B  = 0x09, DAA = 0x27, RLC = 0x07,
	INX_D  = 0x13, DCX_D  = 0x1B, DAD_D  = 0x19, CMA = 0x2F, RRC = 0x0F,
	INX_H  = 0x23, DCX_H  = 0x2B, DAD_H  = 0x29, STC = 0x37, RAL = 0x17,
	INX_SP = 0x33, DCX_SP = 0x3B, DAD_SP = 0x39, CMC = 0x3F, RAR = 0x1F,

	ANA_A = 0xA7, XRA_A = 0xAF, ORA_A = 0xB7, CMP_A = 0xBF, ADI = 0xC6,
	ANA_B = 0xA0, XRA_B = 0xA8, ORA_B = 0xB0, CMP_B = 0xB8, ACI = 0xCE,
	ANA_C = 0xA1, XRA_C = 0xA9, ORA_C = 0xB1, CMP_C = 0xB9, SUI = 0xD6,
	ANA_D = 0xA2, XRA_D = 0xAA, ORA_D = 0xB2, CMP_D = 0xBA, SBI = 0xDE,
	ANA_E = 0xA3, XRA_E = 0xAB, ORA_E = 0xB3, CMP_E = 0xBB, ANI = 0xE6,
	ANA_H = 0xA4, XRA_H = 0xAC, ORA_H = 0xB4, CMP_H = 0xBC, XRI = 0xEE,
	ANA_L = 0xA5, XRA_L = 0xAD, ORA_L = 0xB5, CMP_L = 0xBD, ORI = 0xF6,
	ANA_M = 0xA6, XRA_M = 0xAE, ORA_M = 0xB6, CMP_M = 0xBE, CPI = 0xFE,

	/*====== Branch Control Group ======*/
	JMP  = 0xC3, CALL = 0xCD, RET = 0xC9,
	JNZ  = 0xC2, CNZ  = 0xC4, RNZ = 0xC0,
	JZ   = 0xCA, CZ   = 0xCC, RZ  = 0xC8,
	JNC  = 0xD2, CNC  = 0xD4, RNC = 0xD0,
	JC   = 0xDA, CC   = 0xDC, RC  = 0xD8,
	JPO  = 0xE2, CPO  = 0xE4, RPO = 0xE0,
	JPE  = 0xEA, CPE  = 0xEC, RPE = 0xE8,
	JP   = 0xF2, CP   = 0xF4, RP  = 0xF0,
	JM   = 0xFA, CM   = 0xFC, RM  = 0xF8,

	PCHL = 0xE9,

	/*====== Stack Operations ======*/
	PUSH_B   = 0xC5, POP_B   = 0xC1,
	PUSH_D   = 0xE5, POP_D   = 0xE1,
	PUSH_H   = 0xD5, POP_H   = 0xD1,
	PUSH_PSW = 0xF5, POP_PSW = 0xF1,

	XTHL = 0xE3, SPHL = 0xF9,

	/*====== I/O and Machine Control ======*/
	IN = 0xD3, OUT = 0xDB,
	DI = 0xF3, EI  = 0xFB,

	NOP  = 0x00,
	HLT  = 0x76,

	/*====== Restart ======*/
	RST_0 = 0xC7, RST_1 = 0xCF, RST_2 = 0xD7, RST_3 = 0xDF, RST_4 = 0xE7, RST_5 = 0xEF, RST_6 = 0xF7, RST_7 = 0xFF,
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

void cmp(uint8_t comperand) {
	int result = (int8_t)registers[REG_A] - (int8_t)comperand;
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
	uint16_t SP = 0xFFFF;
	uint8_t TMP = 0x00;
	uint16_t addr = 0;

	g_memory[0x2040] = 5;
	uint8_t numbers[] = { 9, 3, 2, 4, 1 };
	memcpy(g_memory + 0x2041, numbers, sizeof(numbers));
	int iter = 0;
	for (bool running = true; running; iter++) {
		switch (g_memory[PC]) {
			case LDAX_B:
				registers[REG_A] = g_memory[BC_PAIR];
				++PC; break;
			case LDAX_D:
				registers[REG_A] = g_memory[DE_PAIR];
				++PC; break;
			case STAX_B:
				g_memory[BC_PAIR] = registers[REG_A];
				++PC; break;
			case STAX_D:
				g_memory[DE_PAIR] = registers[REG_A];
				++PC; break;
			case LHLD:
				addr = (g_memory[PC + 1] & 0xFF) | ((g_memory[PC + 2] & 0xFF) << 8);
				registers[REG_L] = g_memory[addr];
				registers[REG_H] = g_memory[addr + 1];
				PC += 3; break;
			case SHLD:
				addr = (g_memory[PC + 1] & 0xFF) | ((g_memory[PC + 2] & 0xFF) << 8);
				g_memory[addr]     = registers[REG_L];
				g_memory[addr + 1] = registers[REG_H];
				PC += 3; break;
				break;
			case LDA:
				addr = (g_memory[PC + 1] & 0xFF) | ((g_memory[PC + 2] & 0xFF) << 8);
				registers[REG_A] = g_memory[addr];
				PC += 3; break;
			case STA:
				addr = (g_memory[PC + 1] & 0xFF) | ((g_memory[PC + 2] & 0xFF) << 8);
				g_memory[addr] = registers[REG_A];
				PC += 3; break;
			case XCHG:
				TMP              = registers[REG_L];
				registers[REG_L] = registers[REG_E];
				registers[REG_E] = TMP;
				TMP              = registers[REG_H];
				registers[REG_H] = registers[REG_D];
				registers[REG_D] = TMP;
				++PC; break;
			case LXI_B :
				registers[REG_C] = g_memory[++PC];
				registers[REG_B] = g_memory[++PC];
				++PC;
				break;
			case LXI_D :
				registers[REG_E] = g_memory[++PC];
				registers[REG_D] = g_memory[++PC];
				++PC;
				break;
			case LXI_H :
				registers[REG_L] = g_memory[++PC];
				registers[REG_H] = g_memory[++PC];
				++PC;
				break;
			case LXI_SP:
				SP = g_memory[PC + 1] | (g_memory[PC + 2] << 8);
				PC += 3;
				break;

			case MVI_A:
				registers[REG_A] = g_memory[++PC];
				++PC;
				break;
			case MVI_B:
				registers[REG_B] = g_memory[++PC];
				++PC;
				break;
			case MVI_C:
				registers[REG_C] = g_memory[++PC];
				++PC;
				break;
			case MVI_D:
				registers[REG_D] = g_memory[++PC];
				++PC;
				break;
			case MVI_H:
				registers[REG_H] = g_memory[++PC];
				++PC;
				break;
			case MVI_L:
				registers[REG_L] = g_memory[++PC];
				++PC;
				break;
			case MVI_M:
				g_memory[HL_PAIR] = g_memory[++PC];
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
			case CMP_A: {
				flags &= ~FLAG_S;
				flags |= FLAG_Z;
				PC++;
			} break;
			case CMP_B: {
				cmp(registers[REG_B]);
				PC++;
			} break;
			case CMP_C: {
				cmp(registers[REG_C]);
				PC++;
			} break;
			case CMP_D: {
				cmp(registers[REG_D]);
				PC++;
			} break;
			case CMP_E: {
				cmp(registers[REG_E]);
				PC++;
			} break;
			case CMP_H: {
				cmp(registers[REG_H]);
				PC++;
			} break;
			case CMP_L: {
				cmp(registers[REG_L]);
				PC++;
			} break;
			case CMP_M: {
				cmp(g_memory[HL_PAIR]);
				PC++;
			} break;
			case CPI: {
				cmp(g_memory[++PC]);
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
                ++PC;
				break;

			default: panic("Should be unreachable");
		}

	}

	memcpy(numbers, g_memory + 0x2041, sizeof(numbers));
	return 0;
}

