#include <iostream>


enum Registers 
{
	R0 = 0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	PC, // Programm counter
	COND, // Condition codes
	Registers_Count,
};


uint16_t memory[UINT16_MAX];
uint16_t registers[Registers::Registers_Count];


enum Operations 
{
	BR,
	ADD,
	LD,
	ST,
	JSR,
	AND,
	LDR,
	STR,
	RTI,
	NOT,
	LDI,
	STI,
	JMP,
	RES,
	LEA,
	TRAP,
};


enum Flags 
{
	POS = 1 << 0,
	ZRO = 1 << 1,
	NEG = 1 << 2,
};


uint16_t readMemory(uint16_t addr) 
{
	return memory[addr];
}


void writeMemory(uint16_t addr, uint16_t value)
{
	memory[addr] = value;
}


void updateFlags(uint16_t r0) 
{
	Flags updated = Flags::POS;

	if (registers[r0] >> 15 == 1)
		updated = Flags::NEG;
	else if (registers[r0] == 0)
		updated = Flags::ZRO;

	registers[Registers::COND] = updated;
}


uint16_t signExtend(uint16_t x, int bit_count)
{
	if ((x >> (bit_count - 1)) & 1)
		x |= (0xFFFF << bit_count);
	
	return x;
}


void opAdd(uint16_t instr) 
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t sr1 = (instr >> 6) & 0b111;
	uint16_t sr2 = instr & 0b111;

	uint16_t second = registers[sr2];
	uint16_t immediateFlag = (instr >> 5) & 0b1;
	if (immediateFlag)
	{
		uint16_t imm5 = instr & 0b11111;
		second = signExtend(imm5, 5);
	}

	registers[dr] = registers[sr1] + second;
	updateFlags(dr);
}


void opAnd(uint16_t instr)
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t sr1 = (instr >> 6) & 0b111;
	uint16_t sr2 = instr & 0b111;

	uint16_t second = registers[sr2];
	uint16_t immediateFlag = (instr >> 5) & 0b1;
	if (immediateFlag)
	{
		uint16_t imm5 = instr & 0b11111;
		second = signExtend(imm5, 5);
	}

	registers[dr] = registers[sr1] & second;
	updateFlags(dr);
}


void opNot(uint16_t instr) 
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t sr1 = (instr >> 6) & 0b111;

	registers[dr] = ~registers[sr1];
	updateFlags(dr);
}


void opLd(uint16_t instr) 
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t pcOffset = instr & 0x1ff;

	uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

	registers[dr] = readMemory(addr);
	updateFlags(dr);
}


void opSt(uint16_t instr) 
{
	uint16_t sr = (instr >> 9) & 0b111;
	uint16_t pcOffset = instr & 0x1ff;

	uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

	uint16_t value = registers[sr];
	writeMemory(addr, value);
}


int main() 
{
	std::cout << "Hello, world!\n";
	return 0;
}
