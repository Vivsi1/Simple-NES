#pragma once
#include <cstdint>
#include <array>
#include "Cartridge.h"

class PPU2C02
{
public:
    PPU2C02() = default;

    void connectCartridge(const std::shared_ptr<Cartridge>& cartridge) { this->cartridge = cartridge; }
    void CPUwrite(uint16_t addr, uint8_t data);
    uint8_t CPUread(uint16_t addr);
    uint8_t PPUread(uint16_t addr);
    void PPUwrite(uint16_t addr, uint8_t data);
    std::shared_ptr<Cartridge> cartridge;
    std::array<uint8_t, 0x800> nametable{}; 
    std::array<uint8_t, 0x20> palette{};    
    uint8_t reg[8]{};
};
