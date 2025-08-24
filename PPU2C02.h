#pragma once
#include <cstdint>
#include <array>
#include <memory>

class Cartridge;

class PPU2C02 {
public:
    void connectCartridge(const std::shared_ptr<Cartridge>& c) { cart = c; }
    void    CPUwrite(uint16_t addr, uint8_t data);
    uint8_t CPUread(uint16_t addr);
    uint8_t PPUread(uint16_t addr);
    void    PPUwrite(uint16_t addr, uint8_t data);
    std::array<uint8_t, 0x800> vram{};    
    std::array<uint8_t, 0x20>  palette{}; 
    std::array<uint8_t, 256> oam{};  //just add it for now, its somehow related to foreground rendering
    uint8_t  ppuctrl{};   
    uint8_t  ppumask{};   
    uint8_t  ppustatus{}; 
    uint8_t  oamaddr{};   
    uint8_t  readBuffer{};
    uint16_t v{};   
    uint16_t t{};   
    uint8_t  x{};   
    bool     w{};  
    std::shared_ptr<Cartridge> cart;
    uint16_t mapNametableAddr(uint16_t addr) const; // apply mirroring
    uint16_t incAmount() const { return (ppuctrl & 0x04) ? 32 : 1; }
};
