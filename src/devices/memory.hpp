#include "imemory.hpp"


namespace lc3
{
	class Memory : public IMemory
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


		size_t size() const noexcept override { return UINT16_MAX; }
	};
}
