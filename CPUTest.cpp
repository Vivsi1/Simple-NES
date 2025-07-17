
#include <vector>
#include <unordered_set>
#include <cstdint>
#include "CPU6502.h"
#include "Bus.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
using namespace std;


int main()
{
    std::cout << "Starting 6502 NES test suite (iNES format)..." << std::endl;

    CPU6502 cpu;
    Bus bus;

    // Open the iNES file
    std::ifstream file("nestest.nes", std::ios::binary | std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "❌ Could not open NES ROM file!" << std::endl;
        return 1;
    }

    // Skip 16-byte iNES header
    file.seekg(0x10, std::ios::beg);

    // Read 16KB of PRG-ROM
    const size_t PRG_ROM_SIZE = 0x4000; // 16KB
    std::vector<char> prg_rom(PRG_ROM_SIZE);
    file.read(prg_rom.data(), PRG_ROM_SIZE);
    file.close();
    bus.Initialise();
    for (size_t i = 0; i < PRG_ROM_SIZE; ++i)
    {
        bus.CPUmem[0x8000 + i] = static_cast<uint8_t>(prg_rom[i]);
        bus.CPUmem[0xC000 + i] = static_cast<uint8_t>(prg_rom[i]); // mirror
    }

    // Reset CPU
    cpu.PC = 0xC000;
    cpu.A = 0;
    cpu.X = 0;
    cpu.Y = 0;
    cpu.SP = 0xFD;
    cpu.status.value = 0x24;
    cpu.status.from_byte(cpu.status.value);
    cpu.connectBus(&bus);
    cpu.cycles = 0;
    cpu.totalcycles = 0;

    std::cout << "ROM loaded. Starting execution at $C000\n";

    // Print header for debug output
    std::cout << "PC    OP CODE      A  X  Y  P  SP  CYC" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    // Print initial state
    std::string debug = cpu.debugStr();
    if (!debug.empty())
    {
        std::cout << debug;
    }
    while (true)
    {
        cpu.clock();
        std::string debug = cpu.debugStr();
        cout << debug;
        if (cpu.totalcycles > 30000){
            break;
        }
    }

    std::cout << "\nTest run complete." << std::endl;
    return 0;
}