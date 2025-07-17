#ifndef BUS_H
#define BUS_H

#include <cstdint>
#include "CPU6502.h"
#include "PPU2C02.h"
#include "CARTRIDGE.h"

class Bus
{
public:
    Bus(); 
    void Initialise();                
    uint8_t CPUread(uint16_t addr);      
    void CPUwrite(uint16_t addr, uint8_t data); 
    uint16_t CPUread16(uint16_t addr);    
    uint8_t CPUmem[64 * 1024];   
    uint8_t PPUmem[2 * 1024];     
    CPU6502 cpu;
    PPU2C02 ppu;
    Cartridge cartridge;   
};

#endif 
