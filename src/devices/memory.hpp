#pragma once

#include <iostream>

#include "imemory.hpp"


namespace lc3
{
	class Memory : public IMemory
	{
	private:
		uint16_t* memory = new uint16_t[UINT16_MAX];


		void outOfMemoryError(uint16_t address) const
		{
			std::cerr << "Memory error: Out of memory " << address << ".\n";
			abort();
		}

	public:
		Memory() {}


		Memory(uint16_t* initial, size_t N)
		{
			if (N > size())
				memory = new uint16_t[N];
			
			std::copy(initial, initial + N, memory);
		}


		~Memory()
		{
			delete[] memory;
		}


		uint16_t read(uint16_t address) const override
		{
			if ((size_t)address >= size())
				outOfMemoryError(address);

			return memory[address];
		}


		void write(uint16_t address, uint16_t value) override
		{
			if ((size_t)address >= size())
				outOfMemoryError(address);

			memory[address] = value;
		}


		size_t size() const noexcept override { return UINT16_MAX; }
	};
}
