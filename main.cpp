#include <iostream>
#include <memory>

#include "src/lc3.hpp"


int main() 
{
	std::cout << "The end is near!\n";

	std::unique_ptr<lc3::IMemory> memory(new lc3::Memory);
	lc3::LC3 machine(std::move(memory));
	machine.run();

	return 0;
}
