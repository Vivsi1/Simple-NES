#include "PPU2C02.h"
#include "Cartridge.h"

void PPU2C02::CPUwrite(uint16_t addr, uint8_t data) {
    switch (addr & 0x0007) {
        case 0x2000: // PPUCTRL
            ppuctrl = data;
            t = (t & 0xF3FF) | ((data & 0x03) << 10); 
            break;
        case 0x2001: // PPUMASK
            ppumask = data; break;
        case 0x2003: // OAMADDR
            oamaddr = data; break;
        case 0x2004: // OAMDATA
            // Unimplemented
            break;
        case 0x2005: // PPUSCROLL
            if (!w) { x = data & 0x07; t = (t & 0xFFE0) | (data >> 3); w = true; }
            else     { t = (t & 0x8FFF) | ((data & 0x07) << 12); t = (t & 0xFC1F) | ((data & 0xF8) << 2); w = false; }
            break;
        case 0x2006: // PPUADDR
            if (!w) { t = (t & 0x00FF) | ((uint16_t)(data & 0x3F) << 8); w = true; }
            else     { t = (t & 0xFF00) | data; v = t; w = false; }
            break;
        case 0x2007: // PPUDATA
            PPUwrite(v, data);
            v += incAmount();
            break;
        default: break;
    }
}

uint8_t PPU2C02::CPUread(uint16_t addr) {
    uint8_t data = 0x00;
    switch (addr & 0x2007) {
        case 0x2002: // PPUSTATUS
            data = (ppustatus & 0xE0);
            ppustatus &= ~0x80; 
            w = false;          
            break;
        case 0x2004: // OAMDATA
            // Unimplemented
            break;
        case 0x2007: // PPUDATA (buffered)
        {
            uint16_t a = v & 0x3FFF;
            if (a < 0x3F00) {
                uint8_t buffered = readBuffer;
                readBuffer = PPUread(a);
                data = buffered;
            } else {
                data = PPUread(a); 
                readBuffer = PPUread(a - 0x1000); 
            }
            v += incAmount();
            break;
        }
        default: break;
    }
    return data;
}

uint8_t PPU2C02::PPUread(uint16_t addr) {
    addr &= 0x3FFF;
    uint8_t data = 0x00;

    if (addr <= 0x1FFF) {
        if (cart && cart->PPUread(addr, data)) return data; 
        return 0; 
    }
    else if (addr <= 0x3EFF) {
        uint16_t idx = mapNametableAddr(addr);
        return vram[idx];
    }
    else {
        uint16_t p = addr & 0x001F;
        if (p == 0x10) p = 0x00;
        if (p == 0x14) p = 0x04;
        if (p == 0x18) p = 0x08;
        if (p == 0x1C) p = 0x0C;
        return palette[p];
    }
}

void PPU2C02::PPUwrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        if (cart) cart->PPUwrite(addr, data); 
    }
    else if (addr <= 0x3EFF) {
        uint16_t idx = mapNametableAddr(addr);
        vram[idx] = data;
    }
    else {
        uint16_t p = addr & 0x001F;
        if (p == 0x10) p = 0x00;
        if (p == 0x14) p = 0x04;
        if (p == 0x18) p = 0x08;
        if (p == 0x1C) p = 0x0C;
        palette[p] = data;
    }
}

uint16_t PPU2C02::mapNametableAddr(uint16_t addr) const {
    uint16_t nt = (addr - 0x2000) & 0x0FFF;  
    uint16_t table = (nt / 0x0400) & 0x03;   
    uint16_t offset = nt & 0x03FF;
    auto mir = cart ? cart->getMirror() : Cartridge::MIRROR::VERTICAL;

    uint16_t page =
        (mir == Cartridge::MIRROR::VERTICAL)   ? (table & 0x01)       
      : (mir == Cartridge::MIRROR::HORIZONTAL) ? (table >> 1)        
      : (table & 0x01); 

    return (page * 0x400) + offset; 
}
