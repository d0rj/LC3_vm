#include <cinttypes>


namespace lc3
{
	class Memory 
	{
	private:
		uint16_t memory[UINT16_MAX];

	public:
		uint16_t read(uint16_t address)
		{
			return memory[address];
		}


		void write(uint16_t address, uint16_t value)
		{
			memory[address] = value;
		}


		size_t size() { return UINT16_MAX; }
	};
}
