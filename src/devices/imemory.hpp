#include <cinttypes>


namespace lc3
{
    class IMemory
    {
    public:
        virtual uint16_t read(uint16_t address) const = 0;
        virtual void write(uint16_t address, uint16_t value) = 0;
        virtual size_t size() const = 0;
    };
}
