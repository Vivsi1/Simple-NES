#include "Bus.h"
#include <cstring> 

Bus::Bus()
{
    uint8_t CPUmem[2 * 1024] = {};
    uint8_t PPUmem[2 * 1024] = {};
}

void Bus::Initialise()
{
    for (int i = 0; i < 2 * 1024; i++)
    {
        CPUmem[i] = 0;
    }
    for (int i = 0; i < 2 * 1024; i++)
    {
        PPUmem[i] = 0;
    }
}

void Bus::CPUwrite(uint16_t addr, uint8_t data)
{
    if (addr >= 0x0000 && addr <= 0xFFFF){
        CPUmem[addr % (2 * 1024)] = data;
    }
}

uint8_t Bus::CPUread(uint16_t addr)
{
    if (addr >= 0x0000 && addr <= 0xFFFF)
        return CPUmem[addr % (2 * 1024)];
    return 0x00;
}

uint16_t Bus::CPUread16(uint16_t addr)
{
    uint16_t LoByte = CPUread(addr);
    uint16_t HiByte = CPUread(addr + 1);
    return LoByte | (HiByte << 8);
}