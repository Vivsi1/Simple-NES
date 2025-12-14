#pragma once
#include <cstdint>
#include "../Mirror.h"

class Mapper
{
public:
    Mapper(uint8_t prgBanks, uint8_t chrBanks)
        : nPRGBanks(prgBanks), nCHRBanks(chrBanks) {}
    virtual ~Mapper() = default;
    virtual MIRROR getMirror()
    {
        return MIRROR::HARDWARE;
    };
    virtual bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr) = 0;
    virtual bool cpuMapWrite(uint16_t addr,uint8_t data, uint32_t &mapped_addr) = 0;
    virtual bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) = 0;
    virtual bool ppuMapWrite(uint16_t addr,uint8_t data, uint32_t &mapped_addr) = 0;
    virtual void reset() {}
    virtual bool irqState() { return false; }
    virtual void irqClear() {}

protected:
    uint8_t nPRGBanks = 0;
    uint8_t nCHRBanks = 0;
};
