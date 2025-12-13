#include "PPU2C02.h"
#include "Cartridge.h"
#include "Bus.h"
#include <iostream>

PPU2C02::PPU2C02()
{
    primaryoam.fill(0xFF); // correct NES power-up state
}

void PPU2C02::connectBus(Bus *bus)
{
    this->bus = bus;
}

void PPU2C02::decodeTileToBuffer(uint8_t tile, uint8_t paletteIndex, uint32_t *outPixels)
{
    uint16_t base = ppuctrl.spriteTbl ? 0x1000 : 0x0000;
    uint16_t tileAddr = base + tile * 16;

    for (int y = 0; y < 8; y++)

    {
        uint8_t lo = PPUread(tileAddr + y);
        uint8_t hi = PPUread(tileAddr + y + 8);

        for (int x = 0; x < 8; x++)

        {
            uint8_t bit0 = (lo >> (7 - x)) & 1;
            uint8_t bit1 = (hi >> (7 - x)) & 1;
            uint8_t pixel = (bit1 << 1) | bit0;

            if (pixel == 0)

            {
                outPixels[y * 8 + x] = 0x00000000;
                continue;
            }

            uint8_t paddr = 0x3F00 + paletteIndex * 4 + pixel;
            uint8_t colorIndex = PPUread(paddr) & 0x3F;
            auto rgb = systempalette[colorIndex];
            outPixels[y * 8 + x] =
                (rgb.r << 16) | (rgb.g << 8) | (rgb.b);
        }
    }
}

void PPU2C02::CPUwrite(uint16_t addr, uint8_t data)
{
    switch (addr & 0x0007)

    {
    case 0: // PPUCTRL
        ppuctrl.value = data;
        ppuctrl.from_byte(ppuctrl.value);
        t = (t & 0xF3FF) | ((data & 0x03) << 10);
        w = false;
        break;
    case 1: // PPUMASK
        ppumask.value = data;
        ppumask.from_byte(ppumask.value);
        break;
    case 3: // OAMADDR
        oamaddr = data;
        break;
    case 4: // OAMDATA
        primaryoam[oamaddr] = data;
        oamaddr++;
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
        ppustatus.value &= ~0xC0;
        ppustatus.from_byte(ppustatus.value);
        w = false;
        break;
    case 4: // OAMDATA
        data = primaryoam[oamaddr];
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

void PPU2C02::debugOAMToTexture(uint32_t *outPixels, int w, int h)
{
    // clear output (transparent black)
    int total = w * h;
    for (int i = 0; i < total; ++i)
        outPixels[i] = 0x00000000;

    const int cell = 8; // each sprite cell is 8x8 pixels
    const int cols = 8; // 8 columns of sprites
    const int rows = 8; // 8 rows -> 64 entries

    for (int s = 0; s < 64; ++s)

    {
        const int sx = (s % cols);
        const int sy = (s / cols);
        const int baseX = sx * cell;
        const int baseY = sy * cell;

        uint8_t spriteY = primaryoam[s * 4 + 0];
        uint8_t tile = primaryoam[s * 4 + 1];
        uint8_t attr = primaryoam[s * 4 + 2];
        uint8_t spriteX = primaryoam[s * 4 + 3];

        // If Y == 0xFF many emulators treat as unused/invisible, draw faint bg
        bool inactive = (spriteY == 0xFF);

        // For simplicity always use 8x8 fetch (if you support 8/16 you'd branch here)
        uint16_t tableBase = ppuctrl.spriteTbl ? 0x1000 : 0x0000;
        uint16_t tileAddr = tableBase + (uint16_t(tile) * 16);

        // read 8 rows of pattern bytes
        uint8_t pattern_lo[8];
        uint8_t pattern_hi[8];
        for (int y = 0; y < 8; ++y)

        {
            pattern_lo[y] = PPUread(tileAddr + y);
            pattern_hi[y] = PPUread(tileAddr + y + 8);
        }

        // palette index low bits come from attr & 0x03. Sprite palettes map to 4..7
        uint8_t palIndex = (attr & 0x03) + 4;
        // priority and hflip ignored for debug image, but we can show hflip
        bool hflip = (attr & 0x40) != 0;

        for (int py = 0; py < 8; ++py)

        {
            uint8_t lo = pattern_lo[py];
            uint8_t hi = pattern_hi[py];
            for (int px = 0; px < 8; ++px)

            {
                int bit = 7 - px; // NES pattern bits, MSB = left
                if (hflip)
                    bit = px; // flip horizontally when requested
                uint8_t p0 = (lo >> bit) & 1;
                uint8_t p1 = (hi >> bit) & 1;
                uint8_t colorIdx = (p1 << 1) | p0;

                uint32_t outColor;
                if (inactive)

                {
                    // faint checker or transparent
                    outColor = 0xFF202020; // dark grey for unused slot
                }
                else if (colorIdx == 0)

                {
                    // transparent pixel -> use background color (small checker)
                    bool checker = ((px + py) & 1);
                    outColor = checker ? 0xFF606060 : 0xFF808080;
                }
                else
                {
                    // get the real NES palette color index from PPU palette memory
                    // build the palette address as in drawPixel but local: 0x3F00 | (pal<<2) | colorIdx
                    uint16_t palAddr = 0x3F00 | (palIndex << 2) | colorIdx;
                    uint8_t idx = PPUread(palAddr) & 0x3F;
                    // systempalette[idx] exists in your PPU (Color {r,g,b})
                    auto c = systempalette[idx];
                    outColor = 0xFF000000 | (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | uint32_t(c.b);
                }

                int tx = baseX + px;
                int ty = baseY + py;
                if (tx >= 0 && tx < w && ty >= 0 && ty < h)
                    outPixels[ty * w + tx] = outColor;
            }
        }
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

void PPU2C02::getSpritePixel(uint8_t x, uint8_t &pixel, uint8_t &palette, bool &priority, bool &isSpriteZero)
{
    pixel = 0;
    palette = 0;
    priority = 0;
    isSpriteZero = false;

    for (int i = 0; i < 8; i++)

    {
        auto &s = sprite_shifters[i];
        if (!s.valid)
            continue;
        if (s.valid)

        {
        }

        if (s.x_counter == 0)

        {
            uint8_t p0 = (s.lo & 0x8000) ? 1 : 0;
            uint8_t p1 = (s.hi & 0x8000) ? 1 : 0;
            uint8_t sprPixel = (p1 << 1) | p0;

            if (sprPixel != 0)

            {
                pixel = sprPixel;
                palette = s.palette + 4; // sprite palettes mapped to 4..7
                priority = s.priority;
                isSpriteZero = s.isSpriteZero;

                return;
            }
        }
    }
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
    if (scanline_cycle == -1)
        return; // never scroll on pre-render

    if (!ppumask.showBG && !ppumask.showSprites)
        return;

    if ((v & 0x7000) != 0x7000)
        v += 0x1000;
    else
    {
        v &= ~0x7000;
        int coarseY = (v & 0x03E0) >> 5;

        if (coarseY == 29)

        {
            coarseY = 0;
            v ^= 0x0800;
        }
        else if (coarseY == 31)

        {
            coarseY = 0;
        }
        else
        {
            coarseY++;
        }
        v = (v & ~0x03E0) | (coarseY << 5);
    }
}
void PPU2C02::spriteEvaluation(int dot)
{
    bool odd = dot & 1;
    if (dot == 65)
    {
        sprite_eval.n = 0;                  // Start at primary OAM index 0
        sprite_eval.m = 0;                  // Start at byte 0 (Y-coordinate)
        sprite_eval.found = 0;              // Reset number of found sprites
        sprite_eval.writesDisabled = false; // Allow writing to secondary OAM initially
    }
    if (sprite_eval.n >= 64)
    {
        return; // We do the 0xFF filling
    }
    if (odd)
    {
        int addr = (sprite_eval.n * 4 + sprite_eval.m) & 0xFF; 
        sprite_eval.latch = primaryoam[addr]; // data is read from (primary) OAM
        return;
    }
    if (!sprite_eval.writesDisabled)
    {
        // PHASE 1: Collect up to 8 sprites into secondary OAM
        if (sprite_eval.found < 8)
        {
            // ===== RANGE CHECK ONLY on first byte (Y coordinate) =====
            if (sprite_eval.m == 0)
            {
                uint8_t spriteY = sprite_eval.latch;
                int h = ppuctrl.spriteSize ? 16 : 8;

                // Correct NES rule uses scanline + 1. Insanely bullshit. Dont ever change this or welcome back to off by one hell.
                int diff = (scanline_cycle + 1) - spriteY;
                bool inRange = (diff > 0 && diff <= h);

                if (!inRange)
                {
                    // Sprite not in range. Must SKIP the remaining 3 bytes by advancing to the next sprite (n+1)
                    // The hardware advances the pointer to the start of the next sprite: (n+1, m=0). This seems correct acc to nesdev
                    sprite_eval.n = (sprite_eval.n + 1) & 63;
                    sprite_eval.m = 0;

                    // Check for end of Primary OAM
                    if (sprite_eval.n == 0)
                    {
                        sprite_eval.writesDisabled = true; // No more sprites to evaluate/collect
                    }
                    return;
                }
            }
            secondary_oam.data[sprite_eval.found * 4 + sprite_eval.m] = sprite_eval.latch; //data is written to secondary OAM 

            sprite_eval.m++; // If the value is in range, set the sprite overflow flag in $2002 and read the next 3 entries of OAM (incrementing 'm' after each byte and incrementing 'n' when 'm' overflows)

            if (sprite_eval.m == 4)
            {
                secondary_oam_index[sprite_eval.found] = sprite_eval.n; 

                sprite_eval.m = 0; //overflow
                sprite_eval.found++;
                sprite_eval.n++; //  if m = 3, increment n

                if (sprite_eval.n >= 64)
                    return;
            }
        }
        else
        {
            sprite_eval.writesDisabled = true;
        }
    }
    else
    {
        uint8_t testY = primaryoam[(sprite_eval.n * 4) & 0xFF];
        int h = ppuctrl.spriteSize ? 16 : 8;

        int diff = (scanline_cycle + 1) - testY;
        bool inRange = (diff >= 0 && diff < h);

        if (inRange)
        {
            // Official overflow flag behavior
            ppustatus.spriteOverflow = 1;

            // Perform the 3 dummy increments
            for (int i = 0; i < 3; i++)
            {
                sprite_eval.m++;
                if (sprite_eval.m == 4)
                {
                    sprite_eval.m = 0;
                    sprite_eval.n = (sprite_eval.n + 1) & 63;
                }
            }
        }
        else
        {
            // Normal overflow iteration
            sprite_eval.m = (sprite_eval.m + 1) & 3;
            sprite_eval.n = (sprite_eval.n + 1) & 63;
        }
    }
}

void PPU2C02::fetchSpriteTile(int dot)
{   //I wish I could find better references for this. Lots of guess work and reading.
    int spriteIndex = (dot - 257) / 8;
    int cycle = (dot - 257) % 8;
    if (spriteIndex < 0 || spriteIndex >= 8)
        return;
    auto &entry = sprite_fetch[spriteIndex];
    auto &sh = sprite_shifters[spriteIndex];
    switch (cycle)
    {
    case 0:
        entry.y = secondary_oam.data[spriteIndex * 4 + 0];
        break;
    case 1:
        entry.tile = secondary_oam.data[spriteIndex * 4 + 1];
        break;
    case 2:
        entry.attr = secondary_oam.data[spriteIndex * 4 + 2];
        break;
    case 3:
        entry.x = secondary_oam.data[spriteIndex * 4 + 3];
        entry.isSpriteZero = (secondary_oam_index[spriteIndex] == 0);
        break;
    default:
        break;
    }
    if (cycle != 7)
        return;

    if (entry.y == 0xFF)
    {
        sh.valid = false;
        return;
    }
    int height = ppuctrl.spriteSize ? 16 : 8;

    // IMPORTANT: fetch uses *current scanline*, NOT scanline+1. Insanely bullshit. Dont ever changes this or welcome back to off by one hell.
    int fineY = scanline_cycle - entry.y;

    if (fineY < 0 || fineY >= height)
    {
        sh.valid = false;
        return;
    }

    int tileRow = 0;

    bool flipV = (entry.attr & 0x80) != 0;

    if (ppuctrl.spriteSize) // 8×16 sprites
    {
        tileRow = fineY >> 3;
        if (flipV)
            fineY = 15 - fineY;

        fineY &= 7;
    }
    else // 8×8 sprites
    {
        if (flipV)
            fineY = 7 - fineY;

        fineY &= 7;
    }

    uint16_t tileAddr;
    if (ppuctrl.spriteSize) // 8*16
    {
        uint16_t table = (entry.tile & 1) ? 0x1000 : 0x0000;
        uint8_t tileIndex = (entry.tile & 0xFE) + tileRow;
        tileAddr = table + uint16_t(tileIndex) * 16 + fineY;
    }
    else // 8*8
    {
        uint16_t table = ppuctrl.spriteTbl ? 0x1000 : 0x0000;
        tileAddr = table + uint16_t(entry.tile) * 16 + fineY;
    }
    uint8_t lo = PPUread(tileAddr);
    uint8_t hi = PPUread(tileAddr + 8);

    if (entry.attr & 0x40) // Horizontal Flip
    {
        auto reverse8 = [](uint8_t b)
        {
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            return b;
        };
        lo = reverse8(lo);
        hi = reverse8(hi);
    }
    sh.lo = uint16_t(lo) << 8;
    sh.hi = uint16_t(hi) << 8;
    sh.x_counter = entry.x;
    sh.palette = (entry.attr & 0x03);
    sh.priority = (entry.attr & 0x20) ? 1 : 0;
    sh.isSpriteZero = entry.isSpriteZero;
    sh.valid = true;
}

void PPU2C02::shiftSpriteShifters()
{
    for (auto &s : sprite_shifters)

    {
        if (!s.valid)
            continue;
        if (s.x_counter > 0)

        {
            s.x_counter--;
        }
        else
        {
            s.lo <<= 1;
            s.hi <<= 1;
        }
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
        if (ppuctrl.nmiEnable == 1 && !nmiOccurred)

        {
            // Pass to bus which passes to cpu to nmi?
            nmiOccurred = true;
        }
    }
    else if (scanline_cycle == 261)

    {
        if (dot == 1)

        {
            ppustatus.spriteZeroHit = 0;
            ppustatus.spriteOverflow = 0;
            ppustatus.vblank = 0;
            ppustatus.to_byte();
            nmiOccurred = false;
            frame_complete = true;
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
        if(scanline_cycle == 262){
            scanline_cycle = 0;
        }
    }
}

void PPU2C02::render_scanline()
{
    if (!(ppumask.showBG || ppumask.showSprites) || dot == 0)
        return;

    bool visibleScanline = (scanline_cycle >= 0 && scanline_cycle <= 239);
    bool preScanline = (scanline_cycle == 261);
    bool visibleCycle = (dot >= 1 && dot <= 256);
    bool fetchScanlineCycle = (dot >= 321 && dot <= 336);

    if (visibleScanline && visibleCycle)
    {
        //Helps us choose what to present and when.
        uint16_t mask = 0x8000 >> this->x;
        uint8_t p0 = (bg_shift.pattern_lo & mask) ? 1 : 0;
        uint8_t p1 = (bg_shift.pattern_hi & mask) ? 1 : 0;
        uint8_t pixel = (p1 << 1) | p0;

        uint8_t a0 = (bg_shift.attrib_lo & mask) ? 1 : 0;
        uint8_t a1 = (bg_shift.attrib_hi & mask) ? 1 : 0;
        uint8_t palette = (a1 << 1) | a0;

        uint8_t bgPixel = pixel;
        uint8_t bgPalette = palette;
        bool bgOpaque = (bgPixel != 0);

        if (!ppumask.showBG)

        {
            bgOpaque = false;
            bgPixel = 0;
            bgPalette = 0;
        }

        uint8_t sprPixel = 0;
        uint8_t sprPalette = 0;
        bool sprPriority = 0;
        bool sprIsZero = false;

        if (ppumask.showSprites)

        {
            getSpritePixel(dot - 1, sprPixel, sprPalette, sprPriority, sprIsZero); //Logical enough
        }

        bool sprOpaque = (sprPixel != 0);
        int xcoord = dot - 1;

        if (!ppumask.showLeftBG && xcoord < 8)
            bgOpaque = false;

        if (!ppumask.showLeftSprites && xcoord < 8)
            sprOpaque = false;

        if (sprIsZero && sprOpaque && bgOpaque &&
            xcoord < 255 &&
            ppumask.showBG && ppumask.showSprites &&
            ppustatus.spriteZeroHit == 0 &&
            scanline_cycle >= 0 && scanline_cycle < 240)

        {
            ppustatus.spriteZeroHit = 1;
            ppustatus.to_byte();
        }

        uint8_t finalPixel;
        uint8_t finalPalette;

        if (!bgOpaque && !sprOpaque)

        {
            finalPixel = 0;
            finalPalette = 0;
        }
        else if (!bgOpaque && sprOpaque)

        {
            finalPixel = sprPixel;
            finalPalette = sprPalette;
        }
        else if (bgOpaque && !sprOpaque)

        {
            finalPixel = bgPixel;
            finalPalette = bgPalette;
        }
        else
        {
            if (sprPriority == 0)

            {
                finalPixel = sprPixel;
                finalPalette = sprPalette;
            }
            else
            {
                finalPixel = bgPixel;
                finalPalette = bgPalette;
            }
        }

        drawPixel(dot - 1, scanline_cycle, finalPalette, finalPixel);
    }

    if (scanline_cycle >= 0 && scanline_cycle < 240 and dot < 65)
    {
        if (dot == 1)
        {
            // clear full 32-byte secondary OAM to 0xFF
            for (int i = 0; i < 32; ++i)
                secondary_oam.data[i] = 0xFF;

            // reset sprite evaluation state for this scanline
            sprite_eval.n = 0;
            sprite_eval.m = 0;
            sprite_eval.latch = 0xFF;
            sprite_eval.found = 0;
            sprite_eval.writesDisabled = false;

            // mark indices empty
            for (int i = 0; i < 8; ++i)
                secondary_oam_index[i] = 0xFF;
        }
        if ((dot & 1) == 0)
        {
            int writeIndex = (dot / 2) - 1; // dot=2 -> idx0, dot=4 -> idx1, ..., dot=64 -> idx31?
            if (writeIndex >= 0 && writeIndex < 32)
                secondary_oam.data[writeIndex] = 0xFF; // I assume this is what it meant when it said set to FF in 1-64? Its not specified very well there
        }
    }

    if (dot >= 65 && dot <= 256 && (scanline_cycle >= 0 && scanline_cycle < 240))
    {

        spriteEvaluation(dot); // Cycles 65-256: Sprite evaluation . On odd cycles, data is read from (primary) OAM. On even cycles, data is written to secondary OAM (unless secondary OAM is full, in which case it will read the value in secondary OAM instead)
    }

    if (visibleCycle || (dot >= 321 && dot <= 336))
    {
        shiftBGShifters();
    }

    if (visibleScanline && visibleCycle)
        shiftSpriteShifters(); 

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
            uint16_t tileAddr = (ppuctrl.bgTbl ? 0x1000 : 0x0000) + (bg_latch.nt << 4) + fineY;

            bg_latch.lo = PPUread(tileAddr);
            break;
        }

        case 7: // high pattern byte + X increment
        {
            uint16_t fineY = (v >> 12) & 0x07;
            uint16_t tileAddr = (ppuctrl.bgTbl ? 0x1000 : 0x0000) + (bg_latch.nt << 4) + fineY + 8;

            bg_latch.hi = PPUread(tileAddr);
            incrementScrollX();
            break;
        }

        case 0:
            if (dot >= 8 || preScanline)
                loadBGShifters(); // storeTileData()
            break;
        }
    }
    if (dot == 256)
        incrementScrollY();

    if (dot == 257)
        v = (v & 0x7BE0) | (t & 0x041F);

    if ((ppumask.showSprites) && dot >= 257 && dot <= 320 && scanline_cycle >= 0 && scanline_cycle < 240)
    {
        fetchSpriteTile(dot);
    } // Cycles 257-320: Sprite fetches (8 sprites total, 8 cycles per sprite)  1-4: Read the Y-coordinate, tile number, attributes, and X-coordinate of the selected sprite from secondary OAM

    if (preScanline && (dot >= 280 && dot <= 304))
        v = (v & 0x041F) | (t & 0x7BE0);
}