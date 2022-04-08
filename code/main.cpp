#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define panic(str) assert(0)

#define SET_BIT(flag, b) ((flag) |= (b))
#define RESET_BIT(flag, b) ((flag) &= ~(b))
#define TOGGLE_BIT(flag, b) ((flag) ^= (b))

#define Minimum(a, b) (((a) < (b)) ? (a) : (b))
#define Maximum(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(a, b, v) Minimum(b, Maximum(a, v))

struct String {
	int64_t length;
	uint8_t* data;

	String() : data(0), length(0) {}
	template <int64_t _length>
	constexpr String(const char(&a)[_length]) : data((uint8_t*)a), length(_length - 1) {}
	String(const uint8_t* _data, int64_t _length) : data((uint8_t*)_data), length(_length) {}
	const uint8_t& operator[](const int64_t index) const { assert(index < length); return data[index]; }
	uint8_t& operator[](const int64_t index) { assert(index < length); return data[index]; }
};

enum Registers {
	REG_A = 0,
	REG_F,
	REG_B,
	REG_C,
	REG_D,
	REG_E,
	REG_H,
	REG_L,
	REG_COUNT,
	REG_M,
};

enum Flags : uint8_t {
	FLAG_NONE = 0,
	FLAG_S = 1 << 7,
	FLAG_Z = 1 << 6,
	FLAG_AC = 1 << 4,
	FLAG_P = 1 << 3,
	FLAG_CY = 1,
};

static uint8_t g_memory[64 * 1024];
static uint8_t registers[REG_COUNT] = {};


enum Instruction_Set {
	/*======  data Transfer Group ======*/
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
	MOV_H_M = 0x66, MOV_L_M = 0x6E, XCHG = 0xEB, MVI_M = 0x36,

	LXI_B = 0x01, LDAX_B = 0x0A, STAX_B = 0x02,
	LXI_D = 0x11, LDAX_D = 0x1A, STAX_D = 0x12,
	LXI_H = 0x21, LHLD = 0x2A, SHLD = 0x22,
	LXI_SP = 0x31, LDA = 0x3A, STA = 0x32,

	/*======  Arithmetic and Logic Group ======*/
	ADD_A = 0x87, ADC_A = 0x8F, SUB_A = 0x97, SBB_A = 0x9F, INR_A = 0x3C, DCR_A = 0x3D,
	ADD_B = 0x80, ADC_B = 0x88, SUB_B = 0x90, SBB_B = 0x98, INR_B = 0x04, DCR_B = 0x05,
	ADD_C = 0x81, ADC_C = 0x89, SUB_C = 0x91, SBB_C = 0x99, INR_C = 0x0C, DCR_C = 0x0D,
	ADD_D = 0x82, ADC_D = 0x8A, SUB_D = 0x92, SBB_D = 0x9A, INR_D = 0x14, DCR_D = 0x15,
	ADD_E = 0x83, ADC_E = 0x8B, SUB_E = 0x93, SBB_E = 0x9B, INR_E = 0x1C, DCR_E = 0x1D,
	ADD_H = 0x84, ADC_H = 0x8C, SUB_H = 0x94, SBB_H = 0x9C, INR_H = 0x24, DCR_H = 0x25,
	ADD_L = 0x85, ADC_L = 0x8D, SUB_L = 0x95, SBB_L = 0x9D, INR_L = 0x2C, DCR_L = 0x2D,
	ADD_M = 0x86, ADC_M = 0x8E, SUB_M = 0x96, SBB_M = 0x9E, INR_M = 0x34, DCR_M = 0x35,

	INX_B = 0x03, DCX_B = 0x0B, DAD_B = 0x09, DAA = 0x27, RLC = 0x07,
	INX_D = 0x13, DCX_D = 0x1B, DAD_D = 0x19, CMA = 0x2F, RRC = 0x0F,
	INX_H = 0x23, DCX_H = 0x2B, DAD_H = 0x29, STC = 0x37, RAL = 0x17,
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
	JMP = 0xC3, CALL = 0xCD, RET = 0xC9,
	JNZ = 0xC2, CNZ = 0xC4, RNZ = 0xC0,
	JZ = 0xCA, CZ = 0xCC, RZ = 0xC8,
	JNC = 0xD2, CNC = 0xD4, RNC = 0xD0,
	JC = 0xDA, CC = 0xDC, RC = 0xD8,
	JPO = 0xE2, CPO = 0xE4, RPO = 0xE0,
	JPE = 0xEA, CPE = 0xEC, RPE = 0xE8,
	JP = 0xF2, CP = 0xF4, RP = 0xF0,
	JM = 0xFA, CM = 0xFC, RM = 0xF8,

	PCHL = 0xE9,

	/*====== Stack Operations ======*/
	PUSH_B = 0xC5, POP_B = 0xC1,
	PUSH_D = 0xE5, POP_D = 0xE1,
	PUSH_H = 0xD5, POP_H = 0xD1,
	PUSH_PSW = 0xF5, POP_PSW = 0xF1,

	XTHL = 0xE3, SPHL = 0xF9,

	/*====== I/O and Machine Control ======*/
	IN = 0xD3, OUT = 0xDB,
	DI = 0xF3, EI = 0xFB,

	NOP = 0x00,
	HLT = 0x76,

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

uint8_t update_flag(uint8_t flags, uint16_t previous, uint16_t current, int16_t aux_op) {

	//FLAG Z
	if (current == 0)
		flags = SET_BIT(flags, FLAG_Z);
	else
		flags = RESET_BIT(flags, FLAG_Z);

	//FLAG S
	if (current & 0x80)
		flags = SET_BIT(flags, FLAG_S);
	else
		flags = RESET_BIT(flags, FLAG_S);

	//FLAG CY
	if (current > 0xff)
		flags = SET_BIT(flags, FLAG_CY);
	else
		flags = RESET_BIT(flags, FLAG_CY);

	if ((int16_t)current < 0)
		flags = SET_BIT(flags, FLAG_CY);
	else
		flags = RESET_BIT(flags, FLAG_CY);

	//FLAG P
	int counter = 0;
	for (int i = 0; i < 8; ++i) {
		counter += ((current & (1 << i)) != 0);
	}

	if (counter & 1)
		flags = RESET_BIT(flags, FLAG_P);
	else
		flags = SET_BIT(flags, FLAG_P);

	//FLAG AC
	uint8_t A = previous;
	uint8_t B = (uint8_t)((int8_t)current - (int8_t)previous);

	if ((0xf & A) + (0xf & B) > 0xf)
		flags = SET_BIT(flags, FLAG_AC);
	else
		flags = RESET_BIT(flags, FLAG_AC);

	int16_t nibble = (previous & 0xf);
	if (aux_op < 1) {
		if (nibble < -1 * aux_op)
			flags = SET_BIT(flags, FLAG_AC);
		else
			flags = RESET_BIT(flags, FLAG_AC);
	}
	else if (aux_op > 1) {
		if (nibble + aux_op > 0xf)
			flags = SET_BIT(flags, FLAG_AC);
		else
			flags = RESET_BIT(flags, FLAG_AC);
	}
	return flags;
}

void cmp(uint8_t comperand) {
	int result = (int8_t)registers[REG_A] - (int8_t)comperand;

	registers[REG_F] = update_flag(registers[REG_F], registers[REG_A], result, -(int8_t)comperand);
	/*	if (result > 0) {
			flags &= ~FLAG_CY;
			flags &= ~FLAG_Z;
			flags &= ~FLAG_S;
		} else if (result == 0) {
			flags &= ~FLAG_S;
			flags |= FLAG_Z;
		} else {
			flags |= FLAG_S | FLAG_CY;
			flags &= ~FLAG_Z;
		}*/
}

void push(int rp, uint16_t& sp) {
	g_memory[--sp] = registers[rp];
	g_memory[--sp] = registers[rp + 1];
}

void pop(int rp, uint16_t& sp) {
	registers[rp + 1] = g_memory[sp++];
	registers[rp] = g_memory[sp++];
}

inline int is_char(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

inline int is_num(char c) {
	return (c >= '0' && c <= '9');
}


/*
*
* THE ORDER OF TokenKind and keywords **MUST BE THE SAME** otherwise it'll break **EVERYTHING**
*
*/

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*arr))

inline int StrCompare(String a, String b)
{
	int64_t count = (int64_t)Minimum(a.length, b.length);
	return memcmp(a.data, b.data, count);
}

inline int StrCompareCaseInsensitive(String a, String b)
{
	int64_t count = (int64_t)Minimum(a.length, b.length);
	for (int64_t index = 0; index < count; ++index)
	{
		if (a.data[index] != b.data[index] && a.data[index] + 32 != b.data[index] && a.data[index] != b.data[index])
		{
			return a.data[index] - b.data[index];
		}
	}
	return 0;
}

inline bool StrMatch(String a, String b)
{
	if (a.length != b.length)
		return false;
	return StrCompare(a, b) == 0;
}

inline bool StrMatchCaseInsensitive(String a, String b)
{
	if (a.length != b.length)
		return false;
	return StrCompareCaseInsensitive(a, b) == 0;
}

enum TokenKind {
	TOKEN_MOV, TOKEN_MVI, TOKEN_LXI, TOKEN_ADD, TOKEN_ADC, TOKEN_SBB, TOKEN_SUB,
	TOKEN_ADI, TOKEN_INX, TOKEN_JMP, TOKEN_JNC,


	TOKEN_REG_A, TOKEN_REG_F, TOKEN_REG_B, TOKEN_REG_C,
	TOKEN_REG_D, TOKEN_REG_E, TOKEN_REG_H, TOKEN_REG_L, TOKEN_REG_M,

	_TOKEN_KEYWORD_SEPARATOR,

	TOKEN_ID, TOKEN_COLON, TOKEN_NUMBER, TOKEN_COMMA, TOKEN_ERROR, TOKEN_EOI,



};



static String keywords[] = {
	"MOV", 	"MVI", 	"LXI","ADD", "ADC", "SBB", 	"SUB",
	"ADI", 	"INX", 	"JMP", 	"JNC",
	"A",	"F",	"B",	"C",
	"D",	"E",	"H",	"L",	"M",
};

static_assert(_TOKEN_KEYWORD_SEPARATOR == ARRAY_COUNT(keywords), "You messed up big time, read the comment above");

struct Tokenizer {
	const char* ptr;
	String id;
	uint64_t value;
	TokenKind kind;
};


bool tokenize(Tokenizer* t) {
	const char* ptr = t->ptr;
	while (*ptr) {

		if (*ptr == ' ' || *ptr == '\t') {
			while (*ptr == ' ' || *ptr == '\t') ptr++;
			continue;
		}

		if (*ptr == '\n' || *ptr == '\r') {
			ptr++;
			if (*ptr && *ptr == '\n') ptr++;
			t->kind = TOKEN_EOI;
			t->ptr = ptr;
			return true;
		}

		if (*ptr == ';') {
			while (*ptr && *ptr != '\n' && *ptr != '\r')
				ptr++;
			if (*ptr) ptr++;
			if (*ptr && *ptr == '\n') ptr++;
			t->kind = TOKEN_EOI;
			t->ptr = ptr;
			return true;
		}

		if (is_char(*ptr)) {
			t->id.data = (uint8_t*)ptr++;
			while (is_char(*ptr) || *ptr == '_' || is_num(*ptr)) {
				++ptr;
			}
			t->kind = TOKEN_ID;
			t->id.length = (int64_t)(ptr - (char*)t->id.data);
			for (int i = 0; i < ARRAY_COUNT(keywords); ++i) {
				if (StrMatchCaseInsensitive(t->id, keywords[i])) {
					t->kind = (TokenKind)i;
					break;
				}
			}
			t->ptr = ptr;
			return true;
		}

		if (*ptr == ':') {
			t->kind = TOKEN_COLON;
			t->ptr = ptr + 1;
			return true;
		}

		if (*ptr == ',') {
			t->kind = TOKEN_COMMA;
			t->ptr = ptr + 1;
			return true;
		}

		if (is_num(*ptr)) {
			t->kind = TOKEN_NUMBER;
			const char* start = ptr;
			while (is_num(*ptr)) {
				++ptr;
			}
			if (*ptr == 'H' || *ptr == 'h') {
				t->value = strtoull(start, (char**)&t->ptr, 16);
				t->ptr++;
			}
			else {
				t->value = strtoull(start, (char**)&t->ptr, 10);
			}

			if (errno == ERANGE || t->value > 0xffff) {
				t->kind = TOKEN_ERROR;
				t->id = "Number is out of range";
			}
			return true;
		}

		t->kind = TOKEN_ERROR;
		t->id = "bad character";
		return true;
	}
	return false;
}

Tokenizer create_tokenizer(const char* ptr) {
	Tokenizer result = {};
	result.ptr = ptr;
	return result;
}

int main()
{
	const char* line = R"foo(
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
			)foo";
	Tokenizer tokenizer = create_tokenizer(line);

	while (tokenize(&tokenizer)) {
		if (tokenizer.kind == TOKEN_ERROR) {
			printf("\nERROR: %s\n", tokenizer.id.data);
			break;
		}
		else if (tokenizer.kind == TOKEN_EOI) {
			printf("\n");
		}
		else if (tokenizer.kind == TOKEN_NUMBER) {
			printf("0x%x ", (uint8_t)tokenizer.value);
		}
		else if (tokenizer.kind == TOKEN_COLON) {
			printf(": ");
		}
		else if (tokenizer.kind == TOKEN_COMMA) {
			printf(", ");
		}
		else if (tokenizer.kind == TOKEN_ID) {
			printf("%.*s ", (int32_t)tokenizer.id.length, tokenizer.id.data);
		}
		else {
			printf("%s ", keywords[tokenizer.kind].data);
		}

	}

}

#define UPPER_BYTE_B registers[REG_B]
#define UPPER_BYTE_D registers[REG_D]
#define UPPER_BYTE_H registers[REG_H]
#define UPPER_BYTE_PSW registers[REG_A]

#define LOWER_BYTE_B registers[REG_C]
#define LOWER_BYTE_D registers[REG_E]
#define LOWER_BYTE_H registers[REG_L]
#define LOWER_BYTE_PSW registers[REG_F]

#define REG_PAIR(x) ((uint16_t)UPPER_BYTE_ ##x << 8 | (uint16_t)LOWER_BYTE_ ##x)

#define REGISTER_A registers[REG_A]
#define REGISTER_F registers[REG_F]
#define REGISTER_B registers[REG_B]
#define REGISTER_C registers[REG_C]
#define REGISTER_D registers[REG_D]
#define REGISTER_E registers[REG_E]
#define REGISTER_H registers[REG_H]
#define REGISTER_L registers[REG_L]
#define REGISTER_M g_memory[REG_PAIR(H)]
#define REGISTER(x) REGISTER_ ##x

int main2()
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
		case XTHL:
			TMP = registers[REG_L];
			registers[REG_L] = g_memory[SP];
			g_memory[SP] = TMP;
			TMP = registers[REG_H];
			registers[REG_H] = g_memory[SP + 1];
			g_memory[SP + 1] = TMP;
			++PC; break;
		case SPHL:
			SP = REG_PAIR(H);
			++PC; break;

#define PUSH(rp) \
		case PUSH_ ##rp: \
			g_memory[--SP] = UPPER_BYTE_ ##rp; \
			g_memory[--SP] = LOWER_BYTE_ ##rp; \
			PC++; \
			break

			PUSH(B);
			PUSH(D);
			PUSH(H);
			PUSH(PSW);
#undef PUSH

#define POP(rp) \
		case POP_ ##rp: \
			LOWER_BYTE_ ##rp = g_memory[SP++]; \
			UPPER_BYTE_ ##rp = g_memory[SP++]; \
			PC++; \
			break

			POP(B);
			POP(D);
			POP(H);
			POP(PSW);
#undef POP

		case LDAX_B:
			registers[REG_A] = g_memory[REG_PAIR(B)];
			++PC; break;
		case LDAX_D:
			registers[REG_A] = g_memory[REG_PAIR(D)];
			++PC; break;
		case STAX_B:
			g_memory[REG_PAIR(B)] = registers[REG_A];
			++PC; break;
		case STAX_D:
			g_memory[REG_PAIR(D)] = registers[REG_A];
			++PC; break;
		case LHLD:
			addr = (g_memory[PC + 1] & 0xFF) | ((g_memory[PC + 2] & 0xFF) << 8);
			registers[REG_L] = g_memory[addr];
			registers[REG_H] = g_memory[addr + 1];
			PC += 3; break;
		case SHLD:
			addr = (g_memory[PC + 1] & 0xFF) | ((g_memory[PC + 2] & 0xFF) << 8);
			g_memory[addr] = registers[REG_L];
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
			TMP = registers[REG_L];
			registers[REG_L] = registers[REG_E];
			registers[REG_E] = TMP;
			TMP = registers[REG_H];
			registers[REG_H] = registers[REG_D];
			registers[REG_D] = TMP;
			++PC; break;
		case LXI_B:
			registers[REG_C] = g_memory[++PC];
			registers[REG_B] = g_memory[++PC];
			++PC;
			break;
		case LXI_D:
			registers[REG_E] = g_memory[++PC];
			registers[REG_D] = g_memory[++PC];
			++PC;
			break;
		case LXI_H:
			registers[REG_L] = g_memory[++PC];
			registers[REG_H] = g_memory[++PC];
			++PC;
			break;
		case LXI_SP:
			SP = g_memory[PC + 1] | (g_memory[PC + 2] << 8);
			PC += 3;
			break;

#define MVI(x) \
	case MVI_ ##x : \
		REGISTER(x) = g_memory[++PC]; \
		++PC; \
		break

			MVI(A);
			MVI(B);
			MVI(C);
			MVI(D);
			MVI(E);
			MVI(H);
			MVI(L);
			MVI(M);
#undef MVI

#define MOV(x, y) \
	case MOV_ ##x## _ ##y : \
		REGISTER(x) = REGISTER(y); \
		++PC; \
		break

			MOV(A, A); MOV(B, A); MOV(C, A); MOV(D, A);
			MOV(A, B); MOV(B, B); MOV(C, B); MOV(D, B);
			MOV(A, C); MOV(B, C); MOV(C, C); MOV(D, C);
			MOV(A, D); MOV(B, D); MOV(C, D); MOV(D, D);
			MOV(A, E); MOV(B, E); MOV(C, E); MOV(D, E);
			MOV(A, H); MOV(B, H); MOV(C, H); MOV(D, H);
			MOV(A, L); MOV(B, L); MOV(C, L); MOV(D, L);
			MOV(A, M); MOV(B, M); MOV(C, M); MOV(D, M);

			MOV(E, A); MOV(H, A); MOV(L, A); MOV(M, A);
			MOV(E, B); MOV(H, B); MOV(L, B); MOV(M, B);
			MOV(E, C); MOV(H, C); MOV(L, C); MOV(M, C);
			MOV(E, D); MOV(H, D); MOV(L, D); MOV(M, D);
			MOV(E, E); MOV(H, E); MOV(L, E); MOV(M, E);
			MOV(E, H); MOV(H, H); MOV(L, H); MOV(M, H);
			MOV(E, L); MOV(H, L); MOV(L, L); MOV(M, L);
			MOV(E, M); MOV(H, M); MOV(L, M);
#undef MOV

#define DCR(x) \
		case DCR_ ##x: { \
			registers[REG_F] = update_flag(registers[REG_F], REGISTER(x), (uint16_t)REGISTER(x) - 1, -1); \
			REGISTER(x)--; \
			PC++; \
		} break

			DCR(A);
			DCR(B);
			DCR(C);
			DCR(D);
			DCR(E);
			DCR(H);
			DCR(L);
			DCR(M);
#undef DCR

#define INR(x) \
		case INR_ ##x: { \
			registers[REG_F] = update_flag(registers[REG_F], REGISTER(x), (uint16_t)REGISTER(x) + 1, +1); \
			REGISTER(x)++; \
			PC++; \
		} break

			INR(A);
			INR(B);
			INR(C);
			INR(D);
			INR(E);
			INR(H);
			INR(L);
			INR(M);
#undef INR

#define DCX(ub, lb) \
		case DCX_ ##ub: \
			REGISTER(ub) -= (--REGISTER(lb) == 0xff); \
			PC++; \
			break

			DCX(B, C);
			DCX(D, E);
			DCX(H, L);
#undef DCX

#define INX(ub, lb) \
		case INX_ ##ub: \
			REGISTER(ub) += (++REGISTER(lb) == 0x00); \
			PC++; \
			break

			INX(B, C);
			INX(D, E);
			INX(H, L);
#undef INX

#define CMP(x) \
		case CMP_ ##x: { \
			cmp(REGISTER(x)); \
			PC++; \
		} break

			CMP(A);
			CMP(B);
			CMP(C);
			CMP(D);
			CMP(E);
			CMP(H);
			CMP(L);
			CMP(M);
#undef CMP

		case CPI: {
			cmp(g_memory[++PC]);
			PC++;
		} break;

		case JMP:
			PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1];
			break;

#define JMP_ON_TRUE(flag) \
		PC = (registers[REG_F] & FLAG_ ##flag) ? \
			(uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1] : PC + 3; \
		break

		case JZ : JMP_ON_TRUE(Z);
		case JPE: JMP_ON_TRUE(P);
		case JC : JMP_ON_TRUE(CY);
		case JM : JMP_ON_TRUE(S);
#undef JMP_ON_TRUE

#define JMP_ON_FALSE(flag) \
		PC = (registers[REG_F] & FLAG_ ##flag) ? \
			PC + 3 : (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1]; \
		break

		case JNZ: JMP_ON_FALSE(Z);
		case JPO: JMP_ON_FALSE(P);
		case JNC: JMP_ON_FALSE(CY);
		case JP : JMP_ON_FALSE(S);
#undef JMP_ON_FALSE

		case CALL:
			g_memory[--SP] = PC >> 8;
			g_memory[--SP] = PC & 0xff;
			PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1];
			break;

#define CALL_ON_TRUE(flag) \
		if (registers[REG_F] & FLAG_ ##flag) { \
			g_memory[--SP] = PC >> 8; \
			g_memory[--SP] = PC & 0xff; \
			PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1]; \
		} else { \
			PC += 3;\
		} break

		case CZ : CALL_ON_TRUE(Z);
		case CPE: CALL_ON_TRUE(P);
		case CC : CALL_ON_TRUE(CY);
		case CM : CALL_ON_TRUE(S);
#undef CALL_ON_TRUE

#define CALL_ON_FALSE(flag) \
		if (registers[REG_F] & FLAG_ ##flag) { \
			PC += 3;\
		} else { \
			g_memory[--SP] = PC >> 8; \
			g_memory[--SP] = PC & 0xff; \
			PC = (uint16_t)g_memory[PC + 2] << 8 | g_memory[PC + 1]; \
		} break

		case CNZ: CALL_ON_FALSE(Z);
		case CPO: CALL_ON_FALSE(P);
		case CNC: CALL_ON_FALSE(CY);
		case CP : CALL_ON_FALSE(S);
#undef CALL_ON_FALSE

		case RET:
			PC = (uint16_t)g_memory[SP + 1] << 8 | g_memory[SP];
			SP += 2;
			break;

#define RET_ON_TRUE(flag) \
		if (registers[REG_F] & FLAG_ ##flag) { \
			PC = (uint16_t)g_memory[SP + 1] << 8 | g_memory[SP]; \
			SP += 2; \
		} else { \
			PC += 3; \
		} break

		case RZ : RET_ON_TRUE(Z);
		case RPE: RET_ON_TRUE(P);
		case RC : RET_ON_TRUE(CY);
		case RM : RET_ON_TRUE(S);
#undef RET_ON_TRUE

#define RET_ON_FALSE(flag) \
		if (registers[REG_F] & FLAG_ ##flag) { \
			PC += 3; \
		} else { \
			PC = (uint16_t)g_memory[SP + 1] << 8 | g_memory[SP]; \
			SP += 2; \
		} break

		case RNZ: RET_ON_FALSE(Z);
		case RPO: RET_ON_FALSE(P);
		case RNC: RET_ON_FALSE(CY);
		case RP : RET_ON_FALSE(S);
#undef RET_ON_FALSE

		case PCHL:
			PC = REG_PAIR(H);
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

