#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Mapper.h"

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

    bool imageValid = false;

    std::vector<uint8_t> vPRGMemory;
    std::vector<uint8_t> vCHRMemory;

    std::unique_ptr<Mapper> mapper;
};
