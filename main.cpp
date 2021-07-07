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
	BR, // Conditional Branch
	ADD, // Add or Add Immediate
	LD, // Load
	ST, // Store
	JSR,
	AND, // Binary And or Binary And Immediate
	LDR, // Load Base + Offset
	STR, // Store Base + Offset
	RTI,
	NOT, // Binary Not
	LDI, // Load Indirect
	STI, // Store Indirect
	JMP, // Jump (unconditional branch)
	RES, // Reserved
	LEA, // Load Effective Address
	TRAP,
};


enum Flags 
{
	POS = 1 << 0,
	ZRO = 1 << 1,
	NEG = 1 << 2,
};


enum Trapcodes 
{
	GETC, // Get character from keyboard, not echoed from terminal
	OUT, // Output character
	PUTS, // Output word string
	IN, // Get character form keyboard, echoed from terminal
	PUTSP, // Output byte string
	HALT, // Halt the programm
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


void opLdi(uint16_t instr) 
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t pcOffset = instr & 0x1ff;

	uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

	registers[dr] = readMemory(readMemory(addr));
	updateFlags(dr);
}


void opLdr(uint16_t instr) 
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t br = (instr >> 6) & 0b111;
	uint16_t offset = instr & 0b111111;

	uint16_t addr = registers[br] + signExtend(offset, 6);

	registers[dr] = readMemory(addr);
	updateFlags(dr);
}


void opLea(uint16_t instr) 
{
	uint16_t dr = (instr >> 9) & 0b111;
	uint16_t pcOffset9 = instr & 0x1ff;

	uint16_t addr = registers[Registers::PC] + signExtend(pcOffset9, 9);

	registers[dr] = addr;
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


void opSti(uint16_t instr) 
{
	uint16_t sr = (instr >> 9) & 0b111;
	uint16_t pcOffset = instr & 0x1ff;

	uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

	uint16_t value = registers[sr];
	writeMemory(readMemory(addr), value);
}


void opStr(uint16_t instr) 
{
	uint16_t sr = (instr >> 9) & 0b111;
	uint16_t br = (instr >> 6) & 0b111;
	uint16_t offset = instr & 0b111111;

	uint16_t addr = registers[br] + signExtend(offset, 6);

	writeMemory(addr, registers[sr]);
}


void opBr(uint16_t instr) 
{
	uint16_t cond = (instr >> 9) & 0b111;
	uint16_t offset = instr & 0x1ff;

	if (cond & registers[Registers::COND])
		registers[Registers::PC] += signExtend(offset, 9);
}


void opJmp(uint16_t instr) 
{
	uint16_t r =  (instr >> 6) & 0b111;

	registers[Registers::PC] = registers[r];
}


void opJsr(uint16_t instr) 
{
	registers[Registers::R7] = registers[Registers::PC];
	uint16_t f = (instr >> 11) & 1;

	if (f) // jsr
	{
		uint16_t pcOffset = instr & 0x7ff;
		registers[Registers::PC] += signExtend(pcOffset, 11);
	}
	else // jssr
		registers[Registers::PC] = (instr >> 6) & 0b111;
}


int main() 
{
	std::cout << "Hello, world!\n";
	
	bool isRunning = true;
	while (isRunning)
	{
		uint16_t instr = readMemory(registers[Registers::PC]++);
		uint16_t op = instr >> 12;

		switch (op)
		{
		case Operations::ADD:
			opAdd(instr);
			break;
		case Operations::AND:
			opAnd(instr);
			break;
	   case Operations::BR:
			opBr(instr);
			break;
		case Operations::JMP:
			opJmp(instr);
			break;
		case Operations::JSR:
			opJsr(instr);
			break;
		case Operations::LD:
			opLd(instr);
			break;
		case Operations::LDI:
			opLdi(instr);
			break;
		case Operations::LDR:
			opLdr(instr);
			break;
		case Operations::LEA:
			opLea(instr);
			break;
		case Operations::NOT:
			opNot(instr);
			break;
		case Operations::ST:
			opSt(instr);
			break;
		case Operations::STI:
			opSti(instr);
			break;
		case Operations::STR:
			opStr(instr);
			break;
		case Operations::RES:
		case Operations::RTI:
		default:
			abort();
			break;
		}
	}

	return 0;
}
