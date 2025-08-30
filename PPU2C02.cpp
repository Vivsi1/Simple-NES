#include "PPU2C02.h"
#include "Cartridge.h"
#include "Bus.h"

void PPU2C02::connectBus(Bus *bus)
{
    this->bus = bus;
}

void PPU2C02::CPUwrite(uint16_t addr, uint8_t data) {
    switch (addr & 0x0007) {
        case 0: // PPUCTRL
            ppuctrl.value = data;
            ppuctrl.from_byte(ppuctrl.value);
            t = (t & 0xF3FF) | ((data & 0x03) << 10); 
            break;
        case 1: // PPUMASK
            ppumask.value = data; 
            ppumask.from_byte(ppumask.value);
            break;
        case 3: // OAMADDR
            oamaddr = data; break;
        case 4: // OAMDATA
            oam[oamaddr++] = data;
            break;
        case 5: // PPUSCROLL
            if (!w) { x = data & 0x07; t = (t & 0xFFE0) | (data >> 3); w = true; }
            else     { t = (t & 0x8FFF) | ((data & 0x07) << 12); t = (t & 0xFC1F) | ((data & 0xF8) << 2); w = false; }
            break;
        case 6: // PPUADDR
            if (!w) { t = (t & 0x00FF) | ((uint16_t)(data & 0x3F) << 8); w = true; }
            else     { t = (t & 0xFF00) | data; v = t; w = false; }
            break;
        case 7: // PPUDATA
            PPUwrite(v, data);
            v += incAmount();
            break;
        default: break;
    }
}

uint8_t PPU2C02::CPUread(uint16_t addr) {
    uint8_t data = 0x00;
    switch (addr & 0x0007) {
        case 2: // PPUSTATUS
            data = (ppustatus.value & 0xE0) | (readBuffer & 0x1F);
            ppustatus.value &= ~0x80; 
            ppustatus.from_byte(ppustatus.value);
            w = false;
            break;
        case 4: // OAMDATA
            data = oam[oamaddr];
            break;
        case 7: // PPUDATA (buffered)
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
    if(mir == Cartridge::MIRROR::FOUR_SCREEN){
        vram.resize(4096);
    }
    uint16_t page = 0;
    switch (mir) {
        case Cartridge::MIRROR::VERTICAL:
            page = table & 0x01;
            break;

        case Cartridge::MIRROR::HORIZONTAL:
            page = table >> 1;
            break;

        case Cartridge::MIRROR::ONE_SCREEN_LO:
            page = 0;
            break;

        case Cartridge::MIRROR::ONE_SCREEN_HI:
            page = 1;
            break;

        case Cartridge::MIRROR::FOUR_SCREEN:
            return nt; 
    }
    return (page * 0x400) + offset; 
}

uint16_t PPU2C02::incAmount() { 
    return ppuctrl.increment ? 32 : 1; 
}

void PPU2C02::tick(){
//Rules for scanlines. 0-239 are to use normal render scanline. 
//240 is idle.
//241's 1st cycle is used to set vblank and then request nmi.
//241-260 are clear after this. The PPU makes no memory accesses during these scanlines, so PPU memory can be freely accessed by the program. 
//261 Signifies frame completion. Additionally on 1st scanline clock tick, vblank is cleared and Sprite 0 hit and sprite overflow flags are cleared. Between 280-304 Vertical scroll bits are copied from t into v. This loads the starting scroll position for the upcoming frame.
    if(scanline <= 239){
        render_scanline();
    }
    else if(scanline == 241 and cycle == 1){
        ppustatus.vblank = 1;
        ppustatus.to_byte();
        if(ppuctrl.nmiEnable == 1){
            //Pass to bus which passes to cpu to nmi?
            bus->cpu.nmiRequested = true;
        }
    }
    else if(scanline == 261){
        render_scanline();
        if(cycle == 1){
            ppustatus.spriteZeroHit = 0;
            ppustatus.spriteOverflow = 0;
            ppustatus.vblank = 0;
            ppustatus.to_byte();
        }
        if (cycle >= 280 && cycle <= 304 and (ppumask.showBG or ppumask.showSprites)){
            v = (v & 0x041F) | (t & 0x7BE0);
        }
    }
}

void PPU2C02::render_scanline(){
//Rules for scanline cycles. Cycle 0 is an idle cycle. The value on the PPU address bus during this cycle appears to be the same CHR address that is later used to fetch the low background tile byte starting at dot 5
//Cycles 1-256. The data for each tile is fetched during this phase. Each memory access takes 2 PPU cycles to complete, and 4 must be performed per tile.
//Cycles 257-320. The tile data for the sprites on the next scanline are fetched here. Again, each memory access takes 2 PPU cycles to complete, and 4 are performed for each of the 8 sprites.
//Cycles 321-336. This is where the first two tiles for the next scanline are fetched, and loaded into the shift registers. Again, each memory access takes 2 PPU cycles to complete, and 4 are performed for the two tiles:
//Cycles 337-340. Two bytes are fetched, but the purpose for this is unknown. These fetches are 2 PPU cycles each.
    if(!(ppumask.showBG or ppumask.showSprites) or cycle == 0){
    return;
    }
}