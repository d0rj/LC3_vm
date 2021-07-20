#pragma once

#include <cinttypes>
#include <functional>
#include <map>
#include <format>
#include <string>
#include <memory>

#include "devices/imemory.hpp"
#include "devices/iiodevice.hpp"


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
		std::shared_ptr<IMemory> memory;
		std::shared_ptr<IIODevice> io;

		bool isRunning = false;

		const std::map<Operations, std::function<void(uint16_t)>> handlers = {
			{Operations::ADD, [&](uint16_t instr) {
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
			}},
			{Operations::AND, [&](uint16_t instr) {
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
			}},
			{Operations::NOT, [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t sr1 = (instr >> 6) & 0b111;

				registers[dr] = ~registers[sr1];
				updateFlags(dr);
			}},
			{Operations::LD, [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				registers[dr] = memory->read(addr);
				updateFlags(dr);
			}},
			{Operations::LDI, [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				registers[dr] = memory->read(memory->read(addr));
				updateFlags(dr);
			}},
			{Operations::LDR, [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t br = (instr >> 6) & 0b111;
				uint16_t offset = instr & 0b111111;

				uint16_t addr = registers[br] + signExtend(offset, 6);

				registers[dr] = memory->read(addr);
				updateFlags(dr);
			}},
			{Operations::LEA, [&](uint16_t instr) {
				uint16_t dr = (instr >> 9) & 0b111;
				uint16_t pcOffset9 = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset9, 9);

				registers[dr] = addr;
				updateFlags(dr);
			}},
			{Operations::ST, [&](uint16_t instr) {
				uint16_t sr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				uint16_t value = registers[sr];
				memory->write(addr, value);
			}},
			{Operations::STI, [&](uint16_t instr) {
				uint16_t sr = (instr >> 9) & 0b111;
				uint16_t pcOffset = instr & 0x1ff;

				uint16_t addr = registers[Registers::PC] + signExtend(pcOffset, 9);

				uint16_t value = registers[sr];
				memory->write(memory->read(addr), value);
			}},
			{Operations::STR, [&](uint16_t instr) {
				uint16_t sr = (instr >> 9) & 0b111;
				uint16_t br = (instr >> 6) & 0b111;
				uint16_t offset = instr & 0b111111;

				uint16_t addr = registers[br] + signExtend(offset, 6);

				memory->write(addr, registers[sr]);
			}},
			{Operations::BR, [&](uint16_t instr) {
				uint16_t cond = (instr >> 9) & 0b111;
				uint16_t offset = instr & 0x1ff;

				if (cond & registers[Registers::COND])
					registers[Registers::PC] += signExtend(offset, 9);
			}},
			{Operations::JMP, [&](uint16_t instr) {
				uint16_t r =  (instr >> 6) & 0b111;

				registers[Registers::PC] = registers[r];
			}},
			{Operations::JSR, [&](uint16_t instr) {
				registers[Registers::R7] = registers[Registers::PC];
				uint16_t f = (instr >> 11) & 1;

				if (f) { // jsr 
					uint16_t pcOffset = instr & 0x7ff;
					registers[Registers::PC] += signExtend(pcOffset, 11);
				}
				else // jssr
					registers[Registers::PC] = (instr >> 6) & 0b111;
			}},
			{Operations::RES, [&](uint16_t instr) {
				error("Reserved command " + std::to_string(instr));
			}},
			{Operations::RTI, [&](uint16_t instr) {
				error("Reserved command " + std::to_string(instr));
			}},
			{Operations::TRAP, [&](uint16_t instr) {
				static const std::map<Trapcodes, std::function<void()>> variants = {
					{Trapcodes::GETC, [&]() {
						char input = io->inputChar();
						registers[Registers::R0] = (uint16_t)input;
					}},
					{Trapcodes::OUT, [&]() {
						io->outputChar((char)registers[Registers::R0]);
						io->flush();
					}},
					{Trapcodes::PUTS, [&]() {
						size_t offset = 0;
						uint16_t c = memory->read(registers[Registers::R0]);

						while (c) 
						{
							io->outputChar((char)c);

							++offset;
							c = memory->read(registers[Registers::R0] + offset);
						}

						io->flush();
					}},
					{Trapcodes::IN, [&]() {
						io->outputChars("> ");
						char input = io->inputChar();

						registers[Registers::R0] = (uint16_t)input;
					}},
					{Trapcodes::PUTSP, [&]() {
						size_t offset = 0;
						uint16_t c = memory->read(registers[Registers::R0]);

						while (c)  {
							char first = c & 0xff;
							io->outputChar(first);

							char second = c >> 8;
							if (second)
								io->outputChar(second);

							++offset;
							c = memory->read(registers[Registers::R0]);
						}

						io->flush();
					}},
					{Trapcodes::HALT, [&]() {
						io->outputChars("Stopped.\n");
						isRunning = false;
					}}
				};

				auto key = variants.find((Trapcodes)(instr & 0xff));

				if (key != variants.end())
					key->second();
				else
					error("Unknown TRAP subinstruction " + std::to_string(instr & 0xff));
			}}
		};


		void updateFlags(uint16_t r0) 
		{
			ConditionFlags updated = ConditionFlags::POS;

			if (registers[r0] >> 15 == 1)
				updated = ConditionFlags::NEG;
			else if (registers[r0] == 0)
				updated = ConditionFlags::ZRO;

			registers[Registers::COND] = updated;
		}


		uint16_t signExtend(uint16_t x, int bitCount)
		{
			if ((x >> (bitCount - 1)) & 1)
				x |= (0xffff << bitCount);
			
			return x;
		}


		void error(std::string message) 
		{
			io->errorOutputChars(std::format("Error: {}\n", message));
			isRunning = false;
			abort();
		}


		void setup()
		{
			registers[Registers::PC] = 0x3000;
			isRunning = true;
		}
	public:
		LC3(std::shared_ptr<IMemory> mem, std::shared_ptr<IIODevice> ioDevice)
			: memory(std::move(mem)), io(std::move(ioDevice)) {}


		void run()
		{
			io->outputChars("Setup...\n");
			setup();
			io->outputChars("Done.\nRunning.\n");

			while (isRunning) {
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
