#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include "CPU6502.h"
#include "PPU2C02.h"
#include "CARTRIDGE.h"

class Bus
{
public:
    uint8_t CPUmem[2 * 1024];     
    Bus();
    CPU6502 cpu;
    PPU2C02 ppu;
    std::shared_ptr<Cartridge> cartridge;
    void Initialise();                
    uint8_t CPUread(uint16_t addr);      
    void CPUwrite(uint16_t addr, uint8_t data); 
    uint16_t CPUread16(uint16_t addr);  
    uint8_t PPUread(uint16_t addr);      
    void PPUwrite(uint16_t addr, uint8_t data); 
    void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
};

#endif 
