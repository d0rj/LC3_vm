#include <cinttypes>
#include <functional>
#include <map>
#include <iostream>
#include <string>
#include <memory>

#include "devices/memory.hpp"


namespace lc3
{
	enum Registers : uint16_t
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


	enum Operations : uint16_t
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


	enum ConditionFlags : uint16_t
	{
		POS = 1 << 0,
		ZRO = 1 << 1,
		NEG = 1 << 2,
	};


	enum Trapcodes : uint16_t
	{
		GETC = 0x20, // Get character from keyboard, not echoed from terminal
		OUT = 0x21, // Output character
		PUTS = 0x22, // Output word string
		IN = 0x23, // Get character form keyboard, echoed from terminal
		PUTSP = 0x24, // Output byte string
		HALT = 0x25, // Halt the programm
	};


	class LC3
	{
	private:
		uint16_t registers[Registers::Registers_Count];
		std::unique_ptr<IMemory> memory;

		bool isRunning = false;
		std::function<void(uint16_t)> trapHandler;
		std::map<Operations, std::function<void(uint16_t)>> handlers;


		void updateFlags(uint16_t r0) 
		{
			ConditionFlags updated = ConditionFlags::POS;

			if (registers[r0] >> 15 == 1)
				updated = ConditionFlags::NEG;
			else if (registers[r0] == 0)
				updated = ConditionFlags::ZRO;

			registers[Registers::COND] = updated;
		}


		uint16_t signExtend(uint16_t x, int bit_count)
		{
			if ((x >> (bit_count - 1)) & 1)
				x |= (0xFFFF << bit_count);
			
			return x;
		}


		void error(std::string message) 
		{
			std::cerr << "Error: " << message << std::endl;
			isRunning = false;
			abort();
		}


		// Operations implementation setup
		void init()
		{
			trapHandler = [&](uint16_t instr) {
				static std::map<Trapcodes, std::function<void()>> variants;

				variants[Trapcodes::GETC] = [&]() {
					char input;
					std::cin >> input;

					registers[Registers::R0] = (uint16_t)input;
				};

				variants[Trapcodes::OUT] = [&]() {
					std::cout << (char)registers[Registers::R0] << std::flush;
				};

				variants[Trapcodes::PUTS] = [&]() {
					size_t offset = 0;
					uint16_t c = memory->read(registers[Registers::R0]);

					while (c) 
					{
						std::cout << (char)c;

						++offset;
						c = memory->read(registers[Registers::R0] + offset);
					}

					std::cout << std::flush;
				};

				variants[Trapcodes::IN] = [&]() {
					std::cout << "> ";
					char input;
					std::cin >> input;

					registers[Registers::R0] = (uint16_t)input;
				};

				variants[Trapcodes::PUTSP] = [&]() {
					size_t offset = 0;
					uint16_t c = memory->read(registers[Registers::R0]);

					while (c) 
					{
						char first = c & 0xff;
						std::cout << first;

						char second = c >> 8;
						if (second)
							std::cout << second;

						++offset;
						c = memory->read(registers[Registers::R0]);
					}

					std::cout << std::flush;
				};

				variants[Trapcodes::HALT] = [&]() {
					std::cout << "> Stopped.\n";
					isRunning = false;
				};

				std::map<Trapcodes, std::function<void()>>::iterator key = variants.find((Trapcodes)(instr & 0xff));

				if (key != variants.end())
					key->second();
				else
					error("Unknown TRAP subinstruction " + std::to_string(instr & 0xff));
			};
			handlers[Operations::TRAP] = trapHandler;

			handlers[Operations::ADD] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t sr1 = (instr >> 6) & 0b111;
				uint16_t sr2 = instr & 0b111;

				uint16_t second = registers[sr2];
				uint16_t immediateFlag = (instr >> 5) & 0b1;
				if (immediateFlag) {
					uint16_t imm5 = instr & 0b11111;
					second = signExtend(imm5, 5);
				}

				registers[dr] = registers[sr1] + second;
				updateFlags(dr);
			};

			handlers[Operations::AND] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t sr1 = (instr >> 6) & 0b111;
				uint16_t sr2 = instr & 0b111;

				uint16_t second = registers[sr2];
				uint16_t immediateFlag = (instr >> 5) & 0b1;
				if (immediateFlag) {
					uint16_t imm5 = instr & 0b11111;
					second = signExtend(imm5, 5);
				}

				registers[dr] = registers[sr1] & second;
				updateFlags(dr);
			};

			handlers[Operations::NOT] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t sr1 = (instr >> 6) & 0b111;

				registers[dr] = ~registers[sr1];
				updateFlags(dr);
			};

			handlers[Operations::LD] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				registers[dr] = memory->read(addr);
				updateFlags(dr);
			};

			handlers[Operations::LDI] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				registers[dr] = memory->read(memory->read(addr));
				updateFlags(dr);
			};

			handlers[Operations::LDR] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t br = (instr >> 6) & 0b111;
				uint16_t offset = instr & 0b111111;

				uint16_t addr = registers[br] + signExtend(offset, 6);

				registers[dr] = memory->read(addr);
				updateFlags(dr);
			};

			handlers[Operations::LEA] = [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t pcOffset9 = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset9, 9);

				registers[dr] = addr;
				updateFlags(dr);
			};

			handlers[Operations::ST] = [&](uint16_t instr) {
				uint16_t sr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				uint16_t value = registers[sr];
				memory->write(addr, value);
			};

			handlers[Operations::STI] = [&](uint16_t instr) {
				uint16_t sr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				uint16_t value = registers[sr];
				memory->write(memory->read(addr), value);
			};

			handlers[Operations::STR] = [&](uint16_t instr) {
				uint16_t sr = (instr >> 9) & 0b111;
				uint16_t br = (instr >> 6) & 0b111;
				uint16_t offset = instr & 0b111111;

				uint16_t addr = registers[br] + signExtend(offset, 6);

				memory->write(addr, registers[sr]);
			};

			handlers[Operations::BR] = [&](uint16_t instr) {
				uint16_t cond = (instr >> 9) & 0b111;
				uint16_t offset = instr & 0x1ff;

				if (cond & registers[Registers::COND])
					registers[Registers::PC] += signExtend(offset, 9);
			};

			handlers[Operations::JMP] = [&](uint16_t instr) {
				uint16_t r =  (instr >> 6) & 0b111;

				registers[Registers::PC] = registers[r];
			};

			handlers[Operations::JSR] = [&](uint16_t instr) {
				registers[Registers::R7] = registers[Registers::PC];
				uint16_t f = (instr >> 11) & 1;

				if (f) { // jsr 
					uint16_t pcOffset = instr & 0x7ff;
					registers[Registers::PC] += signExtend(pcOffset, 11);
				}
				else // jssr
					registers[Registers::PC] = (instr >> 6) & 0b111;
			};

			std::function<void(uint16_t)> reservedCommandHandler = [&](uint16_t instr) {
				error("Reserved command " + std::to_string(instr));
			};
			handlers[Operations::RES] = reservedCommandHandler;
			handlers[Operations::RTI] = reservedCommandHandler;
		}


		void setup()
		{
			memory->write(0, 61475);
			memory->write(1, 61475);
			memory->write(2, 61477);

			isRunning = true;
		}
	public:
		LC3(std::unique_ptr<IMemory> mem)
			: memory(std::move(mem)) {}


		void run()
		{
			std::cout << "Initialization...\n";
			init();
			std::cout << "Done.\nSetup...\n";
			setup();
			std::cout << "Done.\nRunning.\n";

			while (isRunning)
			{
				uint16_t instr = memory->read(registers[Registers::PC]++);
				uint16_t op = instr >> 12;

				auto key = handlers.find((Operations)op);

				if (key != handlers.end())
					key->second(instr);
				else
					error("Unknown operation " + std::to_string(op));
			}
		}


		void on()
		{
			isRunning = true;
		}


		void off()
		{
			isRunning = false;
		}
	};
}
