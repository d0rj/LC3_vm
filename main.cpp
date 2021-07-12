#include <iostream>
#include <memory>

#include "src/lc3.hpp"
#include "src/devices/memory.hpp"
#include "src/devices/iodevice.hpp"


int main() 
{
	auto memory = std::make_unique<lc3::Memory>();
	auto io = std::make_unique<lc3::IODevice>();

	lc3::LC3 cpu(
		std::move(memory),
		std::move(io)
		);
	cpu.run();

	return 0;
}
