#pragma once
#include <string>
#include <vector>
#include <memory>
#include "mappers/Mapper.h"
#include "Mirror.h"

class Cartridge
{
public:
    Cartridge() = default;
    Cartridge(const std::string &filename);

    bool isImageValid() const { return imageValid; }

    bool CPUread(uint16_t addr, uint8_t &data);
    bool CPUwrite(uint16_t addr, uint8_t data);
    bool PPUread(uint16_t addr, uint8_t &data);
    bool PPUwrite(uint16_t addr, uint8_t data);


    MIRROR mirror = MIRROR::HORIZONTAL;
    MIRROR getMirror();
    bool imageValid = false;
    std::vector<uint8_t> vPRGMemory; //PGR ROM
    std::vector<uint8_t> vCHRMemory; //CHR ROM
    std::vector<uint8_t> vPRGRAM;  // PRG-RAM/SRAM at 0x6000-0x7FFF

    std::unique_ptr<Mapper> mapper;
};
