#include "Mapper001.h"

Mapper001::Mapper001(uint8_t prgBanks, uint8_t chrBanks)
    : Mapper(prgBanks, chrBanks)
{
    reset();
}

void Mapper001::reset()
{
    loadReg = 0;
    loadCount = 0;

    control = 0x0C;

    chrBank4Lo = 0;
    chrBank4Hi = 0;
    chrBank8 = 0;

    prgBank32 = 0;
    prgBank16Lo = 0;
    prgBank16Hi = nPRGBanks - 1;

    mirrorMode = MIRROR::HARDWARE;
}

bool Mapper001::cpuMapRead(uint16_t addr, uint32_t &mapped_addr)
{
    if (addr < 0x8000)
        return false;

    uint8_t prgMode = (control >> 2) & 0x03;

    if (prgMode == 0 || prgMode == 1)
    {
        // 32 KB mode
        mapped_addr = prgBank32 * 0x8000 + (addr & 0x7FFF);
        return true;
    }
    else if (prgMode == 2)
    {
        // Fix first bank at $8000, switch at $C000
        if (addr < 0xC000)
            mapped_addr = 0 * 0x4000 + (addr & 0x3FFF);
        else
            mapped_addr = prgBank16Hi * 0x4000 + (addr & 0x3FFF);
        return true; // FIXED: Added return
    }
    else
    {
        // Switch at $8000, fix last bank at $C000
        if (addr < 0xC000)
            mapped_addr = prgBank16Lo * 0x4000 + (addr & 0x3FFF);
        else
            mapped_addr = (nPRGBanks - 1) * 0x4000 + (addr & 0x3FFF);
        return true;
    }
}

bool Mapper001::cpuMapWrite(uint16_t addr, uint8_t data, uint32_t &mapped_addr)
{
    if (addr < 0x8000)
        return false;

    if (data & 0x80)
    {
        // Reset shift register
        loadReg = 0;
        loadCount = 0;
        control |= 0x0C;
    }
    else
    {
        loadReg = (loadReg >> 1) | ((data & 1) << 4);
        loadCount++;

        if (loadCount == 5)
        {
            uint8_t target = (addr >> 13) & 0x03;

            switch (target)
            {
            case 0: // Control
                control = loadReg & 0x1F;
                switch (control & 0x03)
                {
                case 0:
                    mirrorMode = MIRROR::ONE_SCREEN_LO;
                    break;
                case 1:
                    mirrorMode = MIRROR::ONE_SCREEN_HI;
                    break;
                case 2:
                    mirrorMode = MIRROR::VERTICAL;
                    break;
                case 3:
                    mirrorMode = MIRROR::HORIZONTAL;
                    break;
                }
                break;

            case 1: // CHR bank 0
                if (control & 0x10)
                    chrBank4Lo = loadReg & ((nCHRBanks * 2) - 1);
                else
                    chrBank8 = loadReg & (nCHRBanks - 1);
                break;

            case 2: // CHR bank 1
                if (control & 0x10)
                    chrBank4Hi = loadReg & ((nCHRBanks * 2) - 1);
                break;

            case 3: // PRG bank
            {
                uint8_t prgMode = (control >> 2) & 0x03;
                if (prgMode == 0 || prgMode == 1)
                    prgBank32 = (loadReg >> 1) & (nPRGBanks > 2 ? (nPRGBanks / 2) - 1 : 0);
                else if (prgMode == 2)
                    prgBank16Hi = loadReg & (nPRGBanks - 1);  // Mode 2: switch $C000
                else
                    prgBank16Lo = loadReg & (nPRGBanks - 1);  // Mode 3: switch $8000
            }
            break;
            }

            loadReg = 0;
            loadCount = 0;
        }
    }

    return true; // mapper handled write, no ROM write
}

bool Mapper001::ppuMapRead(uint16_t addr, uint32_t &mapped_addr)
{
    if (addr >= 0x2000)
        return false;

    // CHR-RAM
    if (nCHRBanks == 0)
    {
        mapped_addr = addr;
        return true;
    }

    if (control & 0x10)
    {
        // 4 KB CHR banks
        if (addr < 0x1000)
            mapped_addr = chrBank4Lo * 0x1000 + (addr & 0x0FFF);
        else
            mapped_addr = chrBank4Hi * 0x1000 + (addr & 0x0FFF);
    }
    else
    {
        // 8 KB CHR bank
        mapped_addr = chrBank8 * 0x2000 + (addr & 0x1FFF);
    }

    return true;
}

bool Mapper001::ppuMapWrite(uint16_t addr, uint8_t data, uint32_t &mapped_addr)
{
    if (addr >= 0x2000)
        return false;

    if (nCHRBanks == 0)
    {
        mapped_addr = addr;
        return true;
    }

    return false;
}

MIRROR Mapper001::getMirror()
{
    return mirrorMode;
}
