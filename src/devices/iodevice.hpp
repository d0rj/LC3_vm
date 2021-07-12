#pragma once

#include <iostream>

#include "iiodevice.hpp"


namespace lc3
{
	class IODevice : public IIODevice
	{
	public:
		char inputChar() const noexcept override
		{
			char result;
			std::cin >> result;
			return result;
		}


		void outputChar(char chr) const noexcept override
		{
			std::cout << chr;
		}


		void outputChars(const char* str) const noexcept override
		{
			std::cout << str;
		}


		void errorOutputChars(const char* str) const noexcept override
		{
			std::cerr << str;
		}


		void flush() const noexcept override
		{
			std::cout << std::flush;
		}
	};
}
