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
    uint8_t nametable[0x800];
    uint8_t palette[0x20];
    Bus();
    CPU6502 cpu;
    PPU2C02 ppu;
    struct OAM_DMA
    {
        bool dummy = true; // alignment/dummy phase
        bool transfer = false;
        uint8_t page = 0x00; // high byte of source address
        uint8_t addr = 0x00; // low byte (0..255)
        uint8_t data = 0x00; // buffered read byte
    };
    uint8_t controller[2];       // latched button states
    uint8_t controller_shift[2]; // shift registers for CPU reads
    uint8_t controller_strobe = 0;
    uint8_t prev_controller_strobe = 0;
    OAM_DMA dma;
    std::shared_ptr<Cartridge> cartridge;
    uint8_t CPUread(uint16_t addr);
    void CPUwrite(uint16_t addr, uint8_t data);
    uint16_t CPUread16(uint16_t addr);
    uint8_t PPUread(uint16_t addr);
    void PPUwrite(uint16_t addr, uint8_t data);
    void insertCartridge(const std::shared_ptr<Cartridge> &cartridge);
    bool stepDMA();
    enum NESButtons
    {
        NES_A = 0x01,
        NES_B = 0x02,
        NES_SELECT = 0x04,
        NES_START = 0x08,
        NES_UP = 0x10,
        NES_DOWN = 0x20,
        NES_LEFT = 0x40,
        NES_RIGHT = 0x80
    };
    void setButton(int controllerIndex, NESButtons btn, bool pressed);
};

#endif
