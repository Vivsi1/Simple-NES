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
    std::cout << "Starting 6502 NES test suite (Cartridge + Mapper)..." << std::endl;

    Bus bus;
    bus.Initialise();
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>("nestest.nes");
    if (!cart->isImageValid()) {
        std::cerr << "Failed to load cartridge!" << std::endl;
        return 1;
    }
    bus.insertCartridge(cart);
    uint8_t lo = 0, hi = 0;
    cart->CPUread(0xFFFC, lo);
    cart->CPUread(0xFFFD, hi);
    uint16_t resetAddr = lo | (hi << 8);
    std::cout << "Reset vector points to: $" << std::hex << resetAddr << "\n";
uint32_t mapped;

cart->mapper->cpuMapRead(0xFFFC, mapped);
lo = cart->vPRGMemory[mapped];

cart->mapper->cpuMapRead(0xFFFD, mapped);
hi = cart->vPRGMemory[mapped];

uint16_t resetVector = lo | (hi << 8);
std::cout << "Reset vector points to: $" << std::hex << resetVector << "\n";


    CPU6502& cpu = bus.cpu;
    cpu.connectBus(&bus);
    cpu.reset();

    std::cout << "ROM loaded via cartridge. Starting execution at \n" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') 
          << cpu.PC << std::endl;
    std::cout << "PC    OP CODE      A  X  Y  P  SP  CYC" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    std::string debug = cpu.debugStr();
    if (!debug.empty()) {
        std::cout << debug;
    }

    while (true) {
        cpu.clock();
        std::string debug = cpu.debugStr();
        std::cout << debug;
        if (cpu.totalcycles > 100) {
            break;
        }
    }

    std::cout << "\nTest run complete." << std::endl;
    return 0;
}
