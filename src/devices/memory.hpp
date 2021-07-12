#pragma once

#include "imemory.hpp"


namespace lc3
{
	class Memory : public IMemory
	{
	private:
		uint16_t* memory = new uint16_t[UINT16_MAX];

	public:
		Memory() {}


		Memory(uint16_t* initial, size_t N)
		{
			std::copy(initial, initial + N, memory);
		}


		~Memory()
		{
			delete[] memory;
		}


		uint16_t read(uint16_t address) const override
		{
			return memory[address];
		}


		void write(uint16_t address, uint16_t value) override
		{
			memory[address] = value;
		}


		size_t size() const noexcept override { return UINT16_MAX; }
	};
}
