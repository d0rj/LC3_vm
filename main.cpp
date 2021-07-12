#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <cinttypes>

#include "src/lc3.hpp"
#include "src/devices/memory.hpp"
#include "src/devices/iodevice.hpp"


std::vector<uint16_t> loadBinary(std::string filename)
{
	std::ifstream input(filename, std::ios::binary);

	std::vector<char> bytes(
		 (std::istreambuf_iterator<char>(input)),
		 (std::istreambuf_iterator<char>()));
	input.close();

	std::vector<uint16_t> result(bytes.size() / 2);

	for (size_t i = 0; i < bytes.size() - 1; i += 2)
		result.push_back((bytes[i + 1] << 8) | bytes[i]);

	return result;
}


int main()
{
	auto program = loadBinary("../programms/programm.raw");

	auto memory = std::make_unique<lc3::Memory>(program.data(), program.size());
	auto io = std::make_unique<lc3::IODevice>();

	lc3::LC3 cpu(
		std::move(memory),
		std::move(io)
		);
	cpu.run();

	return 0;
}
