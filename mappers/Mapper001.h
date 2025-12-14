#pragma once
#include "Mapper.h"
#include "../Mirror.h"

class Mapper001 : public Mapper
{
public:
    Mapper001(uint8_t prgBanks, uint8_t chrBanks);

    bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool cpuMapWrite(uint16_t addr,uint8_t data, uint32_t &mapped_addr) override;
    bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override;
    bool ppuMapWrite(uint16_t addr,uint8_t data, uint32_t &mapped_addr) override;
    MIRROR getMirror() override;
    void reset() override;

private:
    // MMC1 internal registers
    uint8_t loadReg = 0x00;
    uint8_t loadCount = 0;

    uint8_t control = 0x1C;

    uint8_t chrBank4Lo = 0;
    uint8_t chrBank4Hi = 0;
    uint8_t chrBank8 = 0;

    uint8_t prgBank16Lo = 0;
    uint8_t prgBank16Hi = 0;
    uint8_t prgBank32 = 0;

    MIRROR mirrorMode = MIRROR::HARDWARE;
};
