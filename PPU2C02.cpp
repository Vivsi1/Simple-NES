#include "PPU2C02.h"
#include <cstdint>

void PPU2C02::CPUwrite(uint16_t addr, uint8_t data) {
    addr &= 0x0007;
    reg[addr] = data;
}

uint8_t PPU2C02::CPUread(uint16_t addr) {
    addr &= 0x0007;
    return reg[addr];
}

uint8_t PPU2C02::PPUread(uint16_t addr) {
    addr &= 0x3FFF; 
    uint8_t data = 0;

    if(cartridge && cartridge->PPUread(addr, data)) {

    }
    else if(addr >= 0x2000 && addr <= 0x3EFF) {
        data = nametable[addr & 0x07FF];
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF) {
        uint16_t mirrored_addr = addr & 0x001F;
        if(mirrored_addr == 0x10) mirrored_addr = 0x00;
        if(mirrored_addr == 0x14) mirrored_addr = 0x04;
        if(mirrored_addr == 0x18) mirrored_addr = 0x08;
        if(mirrored_addr == 0x1C) mirrored_addr = 0x0C;
        data = palette[mirrored_addr];
    }

    return data;
}

void PPU2C02::PPUwrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF; 

    if(cartridge && cartridge->PPUwrite(addr, data)) {
    }
    else if(addr >= 0x2000 && addr <= 0x3EFF) {
        nametable[addr & 0x07FF] = data;
    }
    else if(addr >= 0x3F00 && addr <= 0x3FFF) {
        uint16_t mirrored_addr = addr & 0x001F;
        if(mirrored_addr == 0x10) mirrored_addr = 0x00;
        if(mirrored_addr == 0x14) mirrored_addr = 0x04;
        if(mirrored_addr == 0x18) mirrored_addr = 0x08;
        if(mirrored_addr == 0x1C) mirrored_addr = 0x0C;
        palette[mirrored_addr] = data;
    }
}
