#include "Cartridge.h"
#include "mappers/Mapper000.h"
#include <fstream>
#include <iostream>

Cartridge::Cartridge(const std::string &filename)
{
    struct sHeader {
        char name[4];
        uint8_t prg_rom_chunks;
        uint8_t chr_rom_chunks;
        uint8_t flags6;
        uint8_t flags7;
        uint8_t prg_ram_size;
        uint8_t flags9;
        uint8_t flags10;
        char padding[5]; 
    } header;


    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs.is_open()) { std::cerr << "Failed to open ROM\n"; return; }

    ifs.read(reinterpret_cast<char*>(&header), 16); 

    if (!(header.name[0] == 'N' && header.name[1] == 'E' &&
          header.name[2] == 'S' && header.name[3] == 0x1A)) {
        std::cerr << "Not a valid iNES file!\n"; return;
    }

    if (header.flags6 & 0x04)
        ifs.seekg(512, std::ios_base::cur); 

    uint8_t mapperID = ((header.flags6 >> 4) | ((header.flags7 >> 4) << 4));

    vPRGMemory.resize(header.prg_rom_chunks * 16384);
    ifs.read(reinterpret_cast<char*>(vPRGMemory.data()), vPRGMemory.size());

    if (header.chr_rom_chunks == 0)
        vCHRMemory.resize(8192);
    else {
        vCHRMemory.resize(header.chr_rom_chunks * 8192);
        ifs.read(reinterpret_cast<char*>(vCHRMemory.data()), vCHRMemory.size());
    }

        if (header.flags6 & 0x08)
    {
        mirror = MIRROR::FOUR_SCREEN;
    }
    else
    {
        mirror = (header.flags6 & 0x01) ? MIRROR::VERTICAL : MIRROR::HORIZONTAL;
    }

    ifs.close();

    switch (mapperID) {
        case 0: mapper = std::make_unique<Mapper000>(header.prg_rom_chunks, header.chr_rom_chunks); break;
        default: std::cerr << "Mapper " << (int)mapperID << " not supported!\n"; return;
    }

    imageValid = true;
    std::cout << "Loaded ROM: " << filename
              << " | Mapper: " << (int)mapperID
              << " | PRG Banks: " << (int)header.prg_rom_chunks
              << " | CHR Banks: " << (int)header.chr_rom_chunks << "\n";
}

Cartridge::MIRROR Cartridge::getMirror(){
    return mirror;
}

bool Cartridge::CPUread(uint16_t addr, uint8_t &data)
{
    uint32_t mapped_addr;
    if (mapper->cpuMapRead(addr, mapped_addr) && mapped_addr < vPRGMemory.size())
    {
        data = vPRGMemory[mapped_addr];
        return true;
    }
    return false;
}

bool Cartridge::CPUwrite(uint16_t addr, uint8_t data)
{
    uint32_t mapped_addr;
    if (mapper->cpuMapWrite(addr, mapped_addr) && mapped_addr < vPRGMemory.size())
    {
        if (mapped_addr < vPRGMemory.size())
            vPRGMemory[mapped_addr] = data;
        return true;
    }
    return false;
}

bool Cartridge::PPUread(uint16_t addr, uint8_t &data)
{
    uint32_t mapped_addr;
    if (mapper->ppuMapRead(addr, mapped_addr) && mapped_addr < vCHRMemory.size())
    {
        data = vCHRMemory[mapped_addr];
        return true;
    }
    return false;
}

bool Cartridge::PPUwrite(uint16_t addr, uint8_t data)
{
    uint32_t mapped_addr;
    if (mapper->ppuMapWrite(addr, mapped_addr) && mapped_addr < vCHRMemory.size())
    {
        vCHRMemory[mapped_addr] = data;
        return true;
    }
    return false;
}
