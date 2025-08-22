#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include "json.hpp"
#include "cpu6502.h"
#include "bus.h"

using json = nlohmann::json;

void printCPUState(const std::string& label, CPU6502& cpu)
{
    std::cout << label << ":\n";
    std::cout << "  PC=" << cpu.PC
              << " A=" << (int)cpu.A
              << " X=" << (int)cpu.X
              << " Y=" << (int)cpu.Y
              << " SP=" << (int)cpu.SP
              << " P=" << (int)cpu.status.value
              << "\n";
}

void printJSONState(const std::string& label, const json& state)
{
    std::cout << label << " (from JSON):\n";
    std::cout << "  PC=" << state["pc"]
              << " A=" << state["a"]
              << " X=" << state["x"]
              << " Y=" << state["y"]
              << " SP=" << state["s"]
              << " P=" << state["p"]
              << "\n";
}

bool runTest(const json &test)
{
    CPU6502 cpu;
    Bus bus;
    bus.Initialise();
    

    // --- Load initial registers ---
    cpu.PC = test["initial"]["pc"];
    cpu.status.value = test["initial"]["p"];
    cpu.A = test["initial"]["a"];
    cpu.X = test["initial"]["x"];
    cpu.Y = test["initial"]["y"];
    cpu.SP = test["initial"]["s"];
    cpu.status.from_byte(cpu.status.value);
    cpu.cycles = 0;

    for (auto &cell : test["initial"]["ram"])
    {
        uint16_t addr = cell[0].get<uint16_t>();
        uint8_t val = cell[1].get<uint8_t>();
        bus.CPUmem[addr] = val;
    }
    cpu.connectBus(&bus);
    cpu.clock();
    do
    {
        cpu.clock();
    } while (cpu.cycles > 0);

    // --- Check final state ---
    bool ok = true;
    auto final = test["final"];
    if (cpu.PC != final["pc"])
        ok = false;
    if (cpu.status.value != final["p"])
        ok = false;
    if (cpu.A != final["a"])
        ok = false;
    if (cpu.X != final["x"])
        ok = false;
    if (cpu.Y != final["y"])
        ok = false;
    if (cpu.SP != final["s"])
        ok = false;

    if (!ok){
        std::cout << "Mismatch detected!\n";
        printJSONState("Expected Final", final);
        printCPUState("CPU After Exec", cpu);
    }
    return ok;
}

int main()
{
    int passed = 0;
    int total = 0;

    // Loop through 256 JSON files (00.json .. FF.json)
    for (int opcode = 0; opcode <= 0xFF; opcode++)
    {
        std::stringstream filename;
        filename << "v1/"
                 << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                 << opcode << ".json";

        std::ifstream f(filename.str());
        if (!f.is_open())
        {
            std::cerr << "Could not open " << filename.str() << "\n";
            continue; // skip missing files
        }

        json tests;
        f >> tests;

        for (auto &t : tests)
        {
            total++;
            if (runTest(t))
                passed++;
            else
                std::cerr << "Failed test in " << filename.str() << " : " << t["name"] << "\n";
        }
    }

    if (total > 0)
    {
        std::cout << "Passed " << passed << " / " << total
                  << " tests (" << (passed * 10000 / total) << "/10000)\n";
    }
    else
    {
        std::cout << "No tests found!\n";
    }
}
