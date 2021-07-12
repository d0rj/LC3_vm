#include <iostream>
#include <memory>

#include "src/lc3.hpp"
#include "src/devices/memory.hpp"
#include "src/devices/iodevice.hpp"


int main() 
{
	std::cout << "The end is near!\n";

	lc3::LC3 machine(
		std::make_unique<lc3::Memory>(),
		std::make_unique<lc3::IODevice>()
		);
	machine.run();

	return 0;
}
