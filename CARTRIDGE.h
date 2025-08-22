#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

class Cartridge{
    public:
        Cartridge() = default;  
        Cartridge(const std::string& sFileName);
};