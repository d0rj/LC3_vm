#pragma once

#include <string>


namespace lc3 
{
	class IIODevice
	{
	public:
		virtual char inputChar() const noexcept = 0;
		virtual void outputChar(char chr) const noexcept = 0;
		virtual void outputChars(const char* str) const noexcept = 0;
		virtual void outputChars(std::string str) const noexcept { outputChars(str.c_str()); }
		virtual void errorOutputChars(const char* str) const noexcept = 0;
		virtual void errorOutputChars(std::string str) const noexcept { errorOutputChars(str.c_str()); }
		virtual void flush() const noexcept = 0;
	};
}
