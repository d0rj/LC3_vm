#include <iostream>
#include <memory>

#include "src/lc3.hpp"
#include "src/devices/memory.hpp"


int main() 
{
	std::cout << "The end is near!\n";

	lc3::LC3 machine(std::make_unique<lc3::Memory>());
	machine.run();

	return 0;
}
