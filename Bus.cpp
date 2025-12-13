#include "Bus.h"
#include <cstring>
#include <iostream>

Bus::Bus()
{
    memset(CPUmem, 0, sizeof(CPUmem));
    memset(nametable, 0, sizeof(nametable));
    memset(palette, 0, sizeof(palette));
    controller[0] = controller[1] = 0;
    controller_shift[0] = controller_shift[1] = 0;
    controller_strobe = 0;
    prev_controller_strobe = 0;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge> &cartridge)
{
    this->cartridge = cartridge;
}

void Bus::CPUwrite(uint16_t addr, uint8_t data)
{
    if (cartridge->CPUwrite(addr, data))
    {
        return;
    }
    else if (addr <= 0x1FFF)
    {
        CPUmem[addr & 0x07FF] = data;
    }
    else if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        ppu.CPUwrite(addr & 0x0007, data);
        return;
    }
    else if (addr == 0x4014)
    {
        dma.page = data;
        dma.addr = 0x00;
        ppu.oamaddr = 0;
        dma.transfer = true;
        dma.dummy = true;
        return;
    }
    else if (addr == 0x4016)
    {
        prev_controller_strobe = controller_strobe;
        controller_strobe = data & 1;

        // Latch on 1 -> 0 transition ONLY
        if (prev_controller_strobe == 1 && controller_strobe == 0)
        {
            controller_shift[0] = controller[0];
            controller_shift[1] = controller[1];
        }
    }
    else if (addr >= 0x4000 && addr <= 0x4017)
    {
        return;
    }
}

uint8_t Bus::CPUread(uint16_t addr)
{
    uint8_t data = 0x00;
    if (cartridge->CPUread(addr, data))
    {
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
    else if (addr == 0x4016 || addr == 0x4017)
    {
        int idx = addr & 1;

        uint8_t value = controller_shift[idx] & 1;

        if (!controller_strobe)
            controller_shift[idx] >>= 1;

        return value | 0x40;
    }
    else if (addr >= 0x4000 && addr <= 0x4017)
    {
        // Unimplemented
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

void Bus::setButton(int controllerIndex, NESButtons btn, bool pressed)
{
    if (pressed)
        controller[controllerIndex] |= btn;
    else
        controller[controllerIndex] &= ~btn;

    // If strobe is active, reload immediately
    if (controller_strobe)
        controller_shift[controllerIndex] = controller[controllerIndex];
}
