#include "Bus.h"
#include <cstring> 


Bus::Bus() {
    memset(CPUmem, 0, sizeof(CPUmem));
    memset(nametable, 0, sizeof(nametable));
    memset(palette, 0, sizeof(palette));
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
    this->cartridge = cartridge;
}

void Bus::CPUwrite(uint16_t addr, uint8_t data)
{
    if (cartridge->CPUwrite(addr, data)) {
        return; 
    }
    else if (addr <= 0x1FFF)
    {
        CPUmem[addr & 0x07FF] = data;  
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        ppu.CPUwrite(addr & 0x0007, data);
    }
    else if (addr >= 0x4000 && addr <= 0x4017)
    {
        //Unimplemented for now
        return;
    }
}

uint8_t Bus::CPUread(uint16_t addr)
{
    uint8_t data = 0x00;
    if (cartridge->CPUread(addr, data)) {
        return data;
    }
    if (addr <= 0x1FFF)
    {
        data = CPUmem[addr & 0x07FF];
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        data = ppu.CPUread(addr & 0x0007);
    }
    else if (addr >= 0x4000 && addr <= 0x4017)
    {
        //Unimplemented
    }
    return data;
}

uint16_t Bus::CPUread16(uint16_t addr)
{
    uint16_t LoByte = CPUread(addr);
    uint16_t HiByte = CPUread(addr + 1);
    return LoByte | (HiByte << 8);
}

uint8_t Bus::PPUread(uint16_t addr)
{
    return ppu.PPUread(addr);
}

void Bus::PPUwrite(uint16_t addr, uint8_t data)
{
    ppu.PPUwrite(addr, data);
}
