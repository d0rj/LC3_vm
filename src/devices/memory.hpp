#include "imemory.hpp"


namespace lc3
{
	class Memory : IMemory
	{
	private:
		uint16_t memory[UINT16_MAX];

	public:
		uint16_t read(uint16_t address) const override
		{
			return memory[address];
		}


		void write(uint16_t address, uint16_t value) override
		{
			memory[address] = value;
		}


		size_t size() const override { return UINT16_MAX; }
	};
}