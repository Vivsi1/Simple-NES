#include "PPU2C02.h"
#include "Cartridge.h"
#include "Bus.h"

void PPU2C02::connectBus(Bus *bus)
{
    this->bus = bus;
}

void PPU2C02::CPUwrite(uint16_t addr, uint8_t data)
{
    switch (addr & 0x0007)
    {
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
        oamaddr = data;
        break;
    case 4: // OAMDATA
        oam[oamaddr++] = data;
        break;
    case 5: // PPUSCROLL
        if (!w)
        {
            x = data & 0x07;
            t = (t & 0xFFE0) | (data >> 3);
            w = true;
        }
        else
        {
            t = (t & 0x8FFF) | ((data & 0x07) << 12);
            t = (t & 0xFC1F) | ((data & 0xF8) << 2);
            w = false;
        }
        break;
    case 6: // PPUADDR
        if (!w)
        {
            t = (t & 0x00FF) | ((uint16_t)(data & 0x3F) << 8);
            w = true;
        }
        else
        {
            t = (t & 0xFF00) | data;
            v = t;
            w = false;
        }
        break;
    case 7: // PPUDATA
        PPUwrite(v, data);
        v += incAmount();
        break;
    default:
        break;
    }
}

uint8_t PPU2C02::CPUread(uint16_t addr)
{
    uint8_t data = 0x00;
    switch (addr & 0x0007)
    {
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
        if (a < 0x3F00)
        {
            uint8_t buffered = readBuffer;
            readBuffer = PPUread(a);
            data = buffered;
        }
        else
        {
            data = PPUread(a);
            readBuffer = PPUread(a - 0x1000);
        }
        v += incAmount();
        break;
    }
    default:
        break;
    }
    return data;
}

uint8_t PPU2C02::PPUread(uint16_t addr)
{
    addr &= 0x3FFF;
    uint8_t data = 0x00;

    if (addr <= 0x1FFF)
    {
        if (cart && cart->PPUread(addr, data))
            return data;
        return 0;
    }
    else if (addr <= 0x3EFF)
    {
        uint16_t idx = mapNametableAddr(addr);
        return vram[idx];
    }
    else
    {
        uint16_t p = addr & 0x001F;
        if (p == 0x10)
            p = 0x00;
        if (p == 0x14)
            p = 0x04;
        if (p == 0x18)
            p = 0x08;
        if (p == 0x1C)
            p = 0x0C;
        return palette[p];
    }
}

void PPU2C02::PPUwrite(uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;

    if (addr <= 0x1FFF)
    {
        if (cart)
            cart->PPUwrite(addr, data);
    }
    else if (addr <= 0x3EFF)
    {
        uint16_t idx = mapNametableAddr(addr);
        vram[idx] = data;
    }
    else
    {
        uint16_t p = addr & 0x001F;
        if (p == 0x10)
            p = 0x00;
        if (p == 0x14)
            p = 0x04;
        if (p == 0x18)
            p = 0x08;
        if (p == 0x1C)
            p = 0x0C;
        palette[p] = data;
    }
}

uint16_t PPU2C02::mapNametableAddr(uint16_t addr) const
{
    uint16_t nt = (addr - 0x2000) & 0x0FFF;
    uint16_t table = (nt / 0x0400) & 0x03;
    uint16_t offset = nt & 0x03FF;
    auto mir = cart ? cart->getMirror() : Cartridge::MIRROR::VERTICAL;
    if (mir == Cartridge::MIRROR::FOUR_SCREEN)
    {
        vram.resize(4096);
    }
    uint16_t page = 0;
    switch (mir)
    {
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

uint16_t PPU2C02::incAmount()
{
    return ppuctrl.increment ? 32 : 1;
}

void PPU2C02::drawPixel(int x, int y, uint8_t palette, uint8_t pixel)
{
    // pixel = background pattern index (0..3)
    // palette = attribute (0..3)

    if (pixel == 0)
    {
        // Background color 0 uses universal background color.
        uint8_t colorIndex = PPUread(0x3F00) & 0x3F;
        auto rgb = systempalette[colorIndex];
        framebuffer[y][x] = rgb;
        return;
    }

    uint16_t addr = 0x3F00 | (palette << 2) | pixel;
    uint8_t colorIndex = PPUread(addr) & 0x3F;
    auto rgb = systempalette[colorIndex];

    framebuffer[y][x] = rgb;
}

void PPU2C02::shiftBGShifters()
{
    bg_shift.pattern_lo <<= 1;
    bg_shift.pattern_hi <<= 1;
    bg_shift.attrib_lo <<= 1;
    bg_shift.attrib_hi <<= 1;
}

void PPU2C02::loadBGShifters()
{
    // Load into LOW byte instead of HIGH byte
    bg_shift.pattern_lo = (bg_shift.pattern_lo & 0xFF00) | bg_latch.lo;
    bg_shift.pattern_hi = (bg_shift.pattern_hi & 0xFF00) | bg_latch.hi;
    
    uint8_t at = bg_latch.at & 0x03;
    uint16_t lo = (at & 0x01) ? 0xFF : 0x00;
    uint16_t hi = (at & 0x02) ? 0xFF : 0x00;
    bg_shift.attrib_lo = (bg_shift.attrib_lo & 0xFF00) | lo;
    bg_shift.attrib_hi = (bg_shift.attrib_hi & 0xFF00) | hi;
}

void PPU2C02::incrementScrollX()
{
    if (!ppumask.showBG && !ppumask.showSprites)
        return;

    if ((v & 0x001F) == 31)
    {
        v &= ~0x001F;
        v ^= 0x0400;
    }
    else
    {
        v++;
    }
}

void PPU2C02::incrementScrollY()
{
    if (scanline_cycle == -1) return; // never scroll on pre-render

    if (!ppumask.showBG && !ppumask.showSprites)
        return;

    if ((v & 0x7000) != 0x7000)
        v += 0x1000;
    else {
        v &= ~0x7000;
        int coarseY = (v & 0x03E0) >> 5;

        if (coarseY == 29) {
            coarseY = 0;
            v ^= 0x0800;
        } else if (coarseY == 31) {
            coarseY = 0;
        } else {
            coarseY++;
        }
        v = (v & ~0x03E0) | (coarseY << 5);
    }
}


void PPU2C02::tick()
{
    // Rules for scanline_cycles. 0-239 are to use normal render scanline_cycle.
    // 240 is idle.
    // 241's 1st dot is used to set vblank and then request nmi.
    // 241-260 are clear after this. The PPU makes no memory accesses during these scanline_cycles, so PPU memory can be freely accessed by the program.
    // 261 Signifies frame completion. Additionally on 1st scanline_cycle clock tick, vblank is cleared and Sprite 0 hit and sprite overflow flags are cleared. Between 280-304 Vertical scroll bits are copied from t into v. This loads the starting scroll position for the upcoming frame.
    if (scanline_cycle <= 239)
    {
        render_scanline();
    }
    else if (scanline_cycle == 241 && dot == 1)
    {
        ppustatus.vblank = 1;
        ppustatus.to_byte();
        if (ppuctrl.nmiEnable == 1)
        {
            // Pass to bus which passes to cpu to nmi?
            bus->cpu.nmiRequested = true;
        }
    }
    else if (scanline_cycle == -1)
    {
        if (dot == 1)
        {
            ppustatus.spriteZeroHit = 0;
            ppustatus.spriteOverflow = 0;
            ppustatus.vblank = 0;
            ppustatus.to_byte();
        }
        render_scanline();
        if (dot >= 280 && dot <= 304 && (ppumask.showBG || ppumask.showSprites))
        {
            v = (v & 0x041F) | (t & 0x7BE0);
        }
    }
    dot++;
    if (dot > 340)
    {
        dot = 0;
        scanline_cycle++;
    }
    if (scanline_cycle >= 261)
    {
        scanline_cycle = -1;
        frame_complete = true;
    }
}

void PPU2C02::render_scanline()
{
    if (!(ppumask.showBG || ppumask.showSprites) || dot == 0)
        return;

    bool visibleScanline     = (scanline_cycle >= 0 && scanline_cycle <= 239);
    bool preScanline         = (scanline_cycle == -1);
    bool visibleCycle        = (dot >= 1 && dot <= 256);
    bool fetchScanlineCycle  = (dot >= 321 && dot <= 336);

    // --------------------------------------------
    // 1. RENDER PIXEL (uses OLD shifter contents)
    // --------------------------------------------
    if (visibleScanline && visibleCycle)
    {
        uint16_t mask = 0x8000 >> x;

        uint8_t p0 = (bg_shift.pattern_lo & mask) ? 1 : 0;
        uint8_t p1 = (bg_shift.pattern_hi & mask) ? 1 : 0;
        uint8_t pixel = (p1 << 1) | p0;

        uint8_t a0 = (bg_shift.attrib_lo & mask) ? 1 : 0;
        uint8_t a1 = (bg_shift.attrib_hi & mask) ? 1 : 0;
        uint8_t palette = (a1 << 1) | a0;

        drawPixel(dot - 1, scanline_cycle, palette, pixel);
    }

    // --------------------------------------------
    // 2. SHIFT SHIFTERS (AFTER rendering, not before)
    // NES shifts at dots 2–257 and 322–337
    // --------------------------------------------
    if ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337))
    {
        shiftBGShifters();
    }

    // --------------------------------------------
    // 3. FETCH TILE DATA
    // --------------------------------------------
    if ((visibleScanline || preScanline) && (visibleCycle || fetchScanlineCycle))
    {
        switch (dot % 8)
        {
        case 1: // nametable
            bg_latch.nt = PPUread(0x2000 | (v & 0x0FFF));
            break;

        case 3: // attribute
        {
            uint16_t addr = 0x23C0 |
                (v & 0x0C00) |
                ((v >> 4) & 0x38) |
                ((v >> 2) & 0x07);

            uint8_t raw = PPUread(addr);
            int coarseX = (v & 0x001F);
            int coarseY = (v & 0x03E0) >> 5;
            int shift = ((coarseY & 2) << 1) | (coarseX & 2);
            bg_latch.at = (raw >> shift) & 0x03;
            break;
        }

        case 5: // low pattern byte
        {
            uint16_t fineY = (v >> 12) & 0x07;
            uint16_t tileAddr = (ppuctrl.bgTbl ? 0x1000 : 0x0000)
                              + (bg_latch.nt << 4)
                              + fineY;

            bg_latch.lo = PPUread(tileAddr);
            break;
        }

        case 7: // high pattern byte + X increment
        {
            uint16_t fineY = (v >> 12) & 0x07;
            uint16_t tileAddr = (ppuctrl.bgTbl ? 0x1000 : 0x0000)
                              + (bg_latch.nt << 4)
                              + fineY + 8;

            bg_latch.hi = PPUread(tileAddr);
            incrementScrollX();
            break;
        }

        case 0:
            if (dot >= 8 || preScanline)
                loadBGShifters();   // storeTileData()
            break;
        }
    }

    // --------------------------------------------
    // 4. SCROLLING / COPY TIMING
    // --------------------------------------------
    if (dot == 256)
        incrementScrollY();

    if (dot == 257)
        v = (v & 0x7BE0) | (t & 0x041F);

    if (preScanline && (dot >= 280 && dot <= 304))
        v = (v & 0x041F) | (t & 0x7BE0);
}
