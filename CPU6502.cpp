#include <string>
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

CPU6502::CPU6502()
{
    using A = CPU6502;
    
    checkpagecross =    {0x10, 0x11, 0x19, 0x1C, 0x1D, 0x30, 0x31, 0x39, 0x3C, 0x3D,
                        0x50, 0x51, 0x59, 0x5C, 0x5D, 0x70, 0x71, 0x79, 0x7C, 0x7D,
                        0x90, 0xB0, 0xB1, 0xB3, 0xB9, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
                        0xD0, 0xD1, 0xD9, 0xDC, 0xDD, 0xF0, 0xF1, 0xF9, 0xFC, 0xFD};
    lookup = {
        {"BRK", &A::BRK, Addressing::IMP, 7},
        {"ORA", &A::ORA, Addressing::IZX, 6},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"SLO", &A::SLO, Addressing::IZX, 8},
        {"NOP", &A::NOP, Addressing::ZP0, 3},
        {"ORA", &A::ORA, Addressing::ZP0, 3},
        {"ASL", &A::ASL, Addressing::ZP0, 5},
        {"SLO", &A::SLO, Addressing::ZP0, 5},
        {"PHP", &A::PHP, Addressing::IMP, 3},
        {"ORA", &A::ORA, Addressing::IMM, 2},
        {"ASL", &A::ASL, Addressing::IMP, 2},
        {"ANC", &A::ANC, Addressing::IMM, 2},
        {"NOP", &A::NOP, Addressing::ABS, 4},
        {"ORA", &A::ORA, Addressing::ABS, 4},
        {"ASL", &A::ASL, Addressing::ABS, 6},
        {"SLO", &A::SLO, Addressing::ABS, 6},
        {"BPL", &A::BPL, Addressing::REL, 2},
        {"ORA", &A::ORA, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"SLO", &A::SLO, Addressing::IZY, 8},
        {"NOP", &A::NOP, Addressing::ZPX, 4},
        {"ORA", &A::ORA, Addressing::ZPX, 4},
        {"ASL", &A::ASL, Addressing::ZPX, 6},
        {"SLO", &A::SLO, Addressing::ZPX, 6},
        {"CLC", &A::CLC, Addressing::IMP, 2},
        {"ORA", &A::ORA, Addressing::ABY, 4},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"SLO", &A::SLO, Addressing::ABY, 7},
        {"NOP", &A::NOP, Addressing::ABX, 4},
        {"ORA", &A::ORA, Addressing::ABX, 4},
        {"ASL", &A::ASL, Addressing::ABX, 7},
        {"SLO", &A::SLO, Addressing::ABX, 7},
        {"JSR", &A::JSR, Addressing::ABS, 6},
        {"AND", &A::AND, Addressing::IZX, 6},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"RLA", &A::RLA, Addressing::IZX, 8},
        {"BIT", &A::BIT, Addressing::ZP0, 3},
        {"AND", &A::AND, Addressing::ZP0, 3},
        {"ROL", &A::ROL, Addressing::ZP0, 5},
        {"RLA", &A::RLA, Addressing::ZP0, 5},
        {"PLP", &A::PLP, Addressing::IMP, 4},
        {"AND", &A::AND, Addressing::IMM, 2},
        {"ROL", &A::ROL, Addressing::IMP, 2},
        {"ANC", &A::ANC, Addressing::IMM, 2},
        {"BIT", &A::BIT, Addressing::ABS, 4},
        {"AND", &A::AND, Addressing::ABS, 4},
        {"ROL", &A::ROL, Addressing::ABS, 6},
        {"RLA", &A::RLA, Addressing::ABS, 6},
        {"BMI", &A::BMI, Addressing::REL, 2},
        {"AND", &A::AND, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"RLA", &A::RLA, Addressing::IZY, 8},
        {"NOP", &A::NOP, Addressing::ZPX, 4},
        {"AND", &A::AND, Addressing::ZPX, 4},
        {"ROL", &A::ROL, Addressing::ZPX, 6},
        {"RLA", &A::RLA, Addressing::ZPX, 6},
        {"SEC", &A::SEC, Addressing::IMP, 2},
        {"AND", &A::AND, Addressing::ABY, 4},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"RLA", &A::RLA, Addressing::ABY, 7},
        {"NOP", &A::NOP, Addressing::ABX, 4},
        {"AND", &A::AND, Addressing::ABX, 4},
        {"ROL", &A::ROL, Addressing::ABX, 7},
        {"RLA", &A::RLA, Addressing::ABX, 7},
        {"RTI", &A::RTI, Addressing::IMP, 6},
        {"EOR", &A::EOR, Addressing::IZX, 6},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"SRE", &A::SRE, Addressing::IZX, 8},
        {"NOP", &A::NOP, Addressing::ZP0, 3},
        {"EOR", &A::EOR, Addressing::ZP0, 3},
        {"LSR", &A::LSR, Addressing::ZP0, 5},
        {"SRE", &A::SRE, Addressing::ZP0, 5},
        {"PHA", &A::PHA, Addressing::IMP, 3},
        {"EOR", &A::EOR, Addressing::IMM, 2},
        {"LSR", &A::LSR, Addressing::IMP, 2},
        {"ALR", &A::ALR, Addressing::IMM, 2},
        {"JMP", &A::JMP, Addressing::ABS, 3},
        {"EOR", &A::EOR, Addressing::ABS, 4},
        {"LSR", &A::LSR, Addressing::ABS, 6},
        {"SRE", &A::SRE, Addressing::ABS, 6},
        {"BVC", &A::BVC, Addressing::REL, 2},
        {"EOR", &A::EOR, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"SRE", &A::SRE, Addressing::IZY, 8},
        {"NOP", &A::NOP, Addressing::ZPX, 4},
        {"EOR", &A::EOR, Addressing::ZPX, 4},
        {"LSR", &A::LSR, Addressing::ZPX, 6},
        {"SRE", &A::SRE, Addressing::ZPX, 6},
        {"CLI", &A::CLI, Addressing::IMP, 2},
        {"EOR", &A::EOR, Addressing::ABY, 4},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"SRE", &A::SRE, Addressing::ABY, 7},
        {"NOP", &A::NOP, Addressing::ABX, 4},
        {"EOR", &A::EOR, Addressing::ABX, 4},
        {"LSR", &A::LSR, Addressing::ABX, 7},
        {"SRE", &A::SRE, Addressing::ABX, 7},
        {"RTS", &A::RTS, Addressing::IMP, 6},
        {"ADC", &A::ADC, Addressing::IZX, 6},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"RRA", &A::RRA, Addressing::IZX, 8},
        {"NOP", &A::NOP, Addressing::ZP0, 3},
        {"ADC", &A::ADC, Addressing::ZP0, 3},
        {"ROR", &A::ROR, Addressing::ZP0, 5},
        {"RRA", &A::RRA, Addressing::ZP0, 5},
        {"PLA", &A::PLA, Addressing::IMP, 4},
        {"ADC", &A::ADC, Addressing::IMM, 2},
        {"ROR", &A::ROR, Addressing::IMP, 2},
        {"ARR", &A::ARR, Addressing::IMM, 2},
        {"JMP", &A::JMP, Addressing::IND, 5},
        {"ADC", &A::ADC, Addressing::ABS, 4},
        {"ROR", &A::ROR, Addressing::ABS, 6},
        {"RRA", &A::RRA, Addressing::ABS, 6},
        {"BVS", &A::BVS, Addressing::REL, 2},
        {"ADC", &A::ADC, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"RRA", &A::RRA, Addressing::IZY, 8},
        {"NOP", &A::NOP, Addressing::ZPX, 4},
        {"ADC", &A::ADC, Addressing::ZPX, 4},
        {"ROR", &A::ROR, Addressing::ZPX, 6},
        {"RRA", &A::RRA, Addressing::ZPX, 6},
        {"SEI", &A::SEI, Addressing::IMP, 2},
        {"ADC", &A::ADC, Addressing::ABY, 4},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"RRA", &A::RRA, Addressing::ABY, 7},
        {"NOP", &A::NOP, Addressing::ABX, 4},
        {"ADC", &A::ADC, Addressing::ABX, 4},
        {"ROR", &A::ROR, Addressing::ABX, 7},
        {"RRA", &A::RRA, Addressing::ABX, 7},
        {"NOP", &A::NOP, Addressing::IMM, 2},
        {"STA", &A::STA, Addressing::IZX, 6},
        {"NOP", &A::NOP, Addressing::IMM, 2},
        {"SAX", &A::SAX, Addressing::IZX, 6},
        {"STY", &A::STY, Addressing::ZP0, 3},
        {"STA", &A::STA, Addressing::ZP0, 3},
        {"STX", &A::STX, Addressing::ZP0, 3},
        {"SAX", &A::SAX, Addressing::ZP0, 3},
        {"DEY", &A::DEY, Addressing::IMP, 2},
        {"NOP", &A::NOP, Addressing::IMM, 2},
        {"TXA", &A::TXA, Addressing::IMP, 2},
        {"XAA", &A::XAA, Addressing::IMM, 2},
        {"STY", &A::STY, Addressing::ABS, 4},
        {"STA", &A::STA, Addressing::ABS, 4},
        {"STX", &A::STX, Addressing::ABS, 4},
        {"SAX", &A::SAX, Addressing::ABS, 4},
        {"BCC", &A::BCC, Addressing::REL, 2},
        {"STA", &A::STA, Addressing::IZY, 6},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"AHX", &A::AHX, Addressing::IZY, 6},
        {"STY", &A::STY, Addressing::ZPX, 4},
        {"STA", &A::STA, Addressing::ZPX, 4},
        {"STX", &A::STX, Addressing::ZPY, 4},
        {"SAX", &A::SAX, Addressing::ZPY, 4},
        {"TYA", &A::TYA, Addressing::IMP, 2},
        {"STA", &A::STA, Addressing::ABY, 5},
        {"TXS", &A::TXS, Addressing::IMP, 2},
        {"TAS", &A::TAS, Addressing::ABY, 5},
        {"NOP", &A::NOP, Addressing::ABX, 5},
        {"STA", &A::STA, Addressing::ABX, 5},
        {"SHX", &A::SHX, Addressing::ABY, 5},
        {"AHX", &A::AHX, Addressing::ABY, 5},
        {"LDY", &A::LDY, Addressing::IMM, 2},
        {"LDA", &A::LDA, Addressing::IZX, 6},
        {"LDX", &A::LDX, Addressing::IMM, 2},
        {"LAX", &A::LAX, Addressing::IZX, 6},
        {"LDY", &A::LDY, Addressing::ZP0, 3},
        {"LDA", &A::LDA, Addressing::ZP0, 3},
        {"LDX", &A::LDX, Addressing::ZP0, 3},
        {"LAX", &A::LAX, Addressing::ZP0, 3},
        {"TAY", &A::TAY, Addressing::IMP, 2},
        {"LDA", &A::LDA, Addressing::IMM, 2},
        {"TAX", &A::TAX, Addressing::IMP, 2},
        {"LAX", &A::LAX, Addressing::IMM, 2},
        {"LDY", &A::LDY, Addressing::ABS, 4},
        {"LDA", &A::LDA, Addressing::ABS, 4},
        {"LDX", &A::LDX, Addressing::ABS, 4},
        {"LAX", &A::LAX, Addressing::ABS, 4},
        {"BCS", &A::BCS, Addressing::REL, 2},
        {"LDA", &A::LDA, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"LAX", &A::LAX, Addressing::IZY, 5},
        {"LDY", &A::LDY, Addressing::ZPX, 4},
        {"LDA", &A::LDA, Addressing::ZPX, 4},
        {"LDX", &A::LDX, Addressing::ZPY, 4},
        {"LAX", &A::LAX, Addressing::ZPY, 4},
        {"CLV", &A::CLV, Addressing::IMP, 2},
        {"LDA", &A::LDA, Addressing::ABY, 4},
        {"TSX", &A::TSX, Addressing::IMP, 2},
        {"LAS", &A::LAS, Addressing::ABY, 4},
        {"LDY", &A::LDY, Addressing::ABX, 4},
        {"LDA", &A::LDA, Addressing::ABX, 4},
        {"LDX", &A::LDX, Addressing::ABY, 4},
        {"LAX", &A::LAX, Addressing::ABY, 4},
        {"CPY", &A::CPY, Addressing::IMM, 2},
        {"CMP", &A::CMP, Addressing::IZX, 6},
        {"NOP", &A::NOP, Addressing::IMM, 2},
        {"DCP", &A::DCP, Addressing::IZX, 8},
        {"CPY", &A::CPY, Addressing::ZP0, 3},
        {"CMP", &A::CMP, Addressing::ZP0, 3},
        {"DEC", &A::DEC, Addressing::ZP0, 5},
        {"DCP", &A::DCP, Addressing::ZP0, 5},
        {"INY", &A::INY, Addressing::IMP, 2},
        {"CMP", &A::CMP, Addressing::IMM, 2},
        {"DEX", &A::DEX, Addressing::IMP, 2},
        {"AXS", &A::AXS, Addressing::IMM, 2},
        {"CPY", &A::CPY, Addressing::ABS, 4},
        {"CMP", &A::CMP, Addressing::ABS, 4},
        {"DEC", &A::DEC, Addressing::ABS, 6},
        {"DCP", &A::DCP, Addressing::ABS, 6},
        {"BNE", &A::BNE, Addressing::REL, 2},
        {"CMP", &A::CMP, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"DCP", &A::DCP, Addressing::IZY, 8},
        {"NOP", &A::NOP, Addressing::ZPX, 4},
        {"CMP", &A::CMP, Addressing::ZPX, 4},
        {"DEC", &A::DEC, Addressing::ZPX, 6},
        {"DCP", &A::DCP, Addressing::ZPX, 6},
        {"CLD", &A::CLD, Addressing::IMP, 2},
        {"CMP", &A::CMP, Addressing::ABY, 4},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"DCP", &A::DCP, Addressing::ABY, 7},
        {"NOP", &A::NOP, Addressing::ABX, 4},
        {"CMP", &A::CMP, Addressing::ABX, 4},
        {"DEC", &A::DEC, Addressing::ABX, 7},
        {"DCP", &A::DCP, Addressing::ABX, 7},
        {"CPX", &A::CPX, Addressing::IMM, 2},
        {"SBC", &A::SBC, Addressing::IZX, 6},
        {"NOP", &A::NOP, Addressing::IMM, 2},
        {"ISC", &A::ISC, Addressing::IZX, 8},
        {"CPX", &A::CPX, Addressing::ZP0, 3},
        {"SBC", &A::SBC, Addressing::ZP0, 3},
        {"INC", &A::INC, Addressing::ZP0, 5},
        {"ISC", &A::ISC, Addressing::ZP0, 5},
        {"INX", &A::INX, Addressing::IMP, 2},
        {"SBC", &A::SBC, Addressing::IMM, 2},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"SBC", &A::SBC, Addressing::IMM, 2},
        {"CPX", &A::CPX, Addressing::ABS, 4},
        {"SBC", &A::SBC, Addressing::ABS, 4},
        {"INC", &A::INC, Addressing::ABS, 6},
        {"ISC", &A::ISC, Addressing::ABS, 6},
        {"BEQ", &A::BEQ, Addressing::REL, 2},
        {"SBC", &A::SBC, Addressing::IZY, 5},
        {"KIL", &A::KIL, Addressing::IMP, 2},
        {"ISC", &A::ISC, Addressing::IZY, 8},
        {"NOP", &A::NOP, Addressing::ZPX, 4},
        {"SBC", &A::SBC, Addressing::ZPX, 4},
        {"INC", &A::INC, Addressing::ZPX, 6},
        {"ISC", &A::ISC, Addressing::ZPX, 6},
        {"SED", &A::SED, Addressing::IMP, 2},
        {"SBC", &A::SBC, Addressing::ABY, 4},
        {"NOP", &A::NOP, Addressing::IMP, 2},
        {"ISC", &A::ISC, Addressing::ABY, 7},
        {"NOP", &A::NOP, Addressing::ABX, 4},
        {"SBC", &A::SBC, Addressing::ABX, 4},
        {"INC", &A::INC, Addressing::ABX, 7},
        {"ISC", &A::ISC, Addressing::ABX, 7},
    };
}

void CPU6502::connectBus(Bus *bus)
{
    this->bus = bus;
}

uint8_t CPU6502::read(uint16_t addr)
{
    return bus->CPUread(addr);
}

uint16_t CPU6502::read16(std::uint16_t addr)
{
    return bus->CPUread16(addr);
}

void CPU6502::write(std::uint16_t addr, std::uint8_t data)
{
    bus->CPUwrite(addr, data);
}

void CPU6502::push(std::uint8_t data)
{
    bus->CPUwrite(0x0100 + SP--, data);
}

void CPU6502::push16(std::uint16_t data)
{
    push((data >> 8) & 0x00FF);
    push(data & 0x00FF);
}

uint8_t CPU6502::pop()
{
    return read(0x0100 + ++SP);
}

uint16_t CPU6502::pop16()
{
    std::uint16_t l = pop();
    std::uint16_t h = pop();

    return (h << 8) | l;
}

void CPU6502::clock()
{
    if (nmiRequested) {
        nmi();
        nmiRequested = false;
    }
    if (cycles == 0)
    {
        execute();
    }

    cycles--;
}

void CPU6502::reset()
{
    PC = read16(0xFFFC);
    A = 0;
    X = 0;
    Y = 0;
    SP = 0xFD;
    status.value = 0x24;
    cycles = 7;

}

void CPU6502::nmi()
{
    push16(PC);
    push((status.value & ~0x30) | 0x20);
    status.i = 1;
    PC = read16(0xFFFA);
    cycles = 8;
}

void CPU6502::irq()
{
    if (status.i == 0)
    {
        push16(PC);
        push((status.value & ~0x30) | 0x20);
        status.i = 1;
        PC = read16(0xFFFE);
        cycles = 7;
    }
}


void CPU6502::serialize(std::ostream &os) const
{
    auto begin = reinterpret_cast<const char *>(this) + offsetof(CPU6502, PC);
    auto end = reinterpret_cast<const char *>(this) + offsetof(CPU6502, bus);
    os.write(begin, end - begin);
}

void CPU6502::deserialize(std::istream &is)
{
    auto begin = reinterpret_cast<char *>(this) + offsetof(CPU6502, PC);
    auto end = reinterpret_cast<char *>(this) + offsetof(CPU6502, bus);
    is.read(begin, end - begin);
}

void CPU6502::setPc(std::uint16_t newPc)
{
    PC = newPc;
}

bool CPU6502::isCrossed(uint16_t A, int16_t b)
{
    return (A & 0xFF00) != (b & 0xFF00);
}

void CPU6502::execute()
{
    uint8_t opcode = read(PC++);
    uint16_t address = 0;
    bool pageCrossed = false;
    const instruction &ins = lookup[opcode];

    switch (ins.addressing)
    {
    case Addressing::IMP: // Implied
    case Addressing::ACC: // Accumulator
        break;

    case Addressing::IMM: // Immediate
        address = PC++;
        break;

    case Addressing::ZP0: // Zero Page
        address = read(PC++) & 0x00FF;
        break;

    case Addressing::ZPX: // Zero Page,X
        address = (read(PC++) + X) & 0x00FF;
        break;

    case Addressing::ZPY: // Zero Page,Y
        address = (read(PC++) + Y) & 0x00FF;
        break;

    case Addressing::REL: // Relative
        address = read(PC++);
        if (address & 0x80) // Sign extend if negative
        {
            address |= 0xFF00;
        }
        break;

    case Addressing::ABS: // Absolute
        address = read16(PC);
        PC += 2;
        break;

    case Addressing::ABX: // Absolute,X
    {
        uint16_t base = read16(PC);
        address = base + X;
        PC += 2;
        pageCrossed = isCrossed(base, address);
    }
    break;

    case Addressing::ABY: // Absolute,Y
    {
        uint16_t base = read16(PC);
        address = base + Y;
        PC += 2;
        pageCrossed = isCrossed(base, address);
    }
    break;

    case Addressing::IND: // Indirect
    {
        uint16_t ptr = read16(PC);
        PC += 2;

        // Simulate 6502 page boundary hardware bug
        if ((ptr & 0x00FF) == 0x00FF)
        {
            // If pointer is at page boundary (e.g. $xxFF)
            // Read LSB from $xxFF and MSB from $xx00 (not $xx+1:00)
            address = (read(ptr & 0xFF00) << 8) | read(ptr);
        }
        else
        {
            // Normal case
            address = (read(ptr + 1) << 8) | read(ptr);
        }
    }
    break;

    case Addressing::IZX: // (Indirect,X)
    {
        uint8_t temp = read(PC++);
        uint8_t lo = read((temp + X) & 0x00FF);
        uint8_t hi = read((temp + X + 1) & 0x00FF);
        address = (hi << 8) | lo;
    }
    break;

    case Addressing::IZY: // (Indirect),Y
    {
        uint8_t temp = read(PC++);
        uint8_t lo = read(temp & 0x00FF);
        uint8_t hi = read((temp + 1) & 0x00FF);
        uint16_t base = (hi << 8) | lo;
        address = base + Y;
        pageCrossed = isCrossed(base, address);
    }
    break;
    }
    if (opcode == 0x0A) // ASL A
    {
        status.c = (A >> 7) & 1;
        A <<= 1;
        status.z = (A == 0 ? 1 : 0);
        status.n = (((A >> 7) & 1) == 1 ? 1 : 0);
    }
    else if (opcode == 0x2A) // ROL A
    {
        uint8_t oldC = status.c;
        status.c = (A >> 7) & 1;
        A = (A << 1) | oldC;
        status.z = (A == 0 ? 1 : 0);
        status.n = (((A >> 7) & 1) == 1 ? 1 : 0);
    }
    else if (opcode == 0x4A) // LSR A
    {
        status.c = A & 1;
        A >>= 1;
        status.z = (A == 0 ? 1 : 0);
        status.n = (((A >> 7) & 1) == 1 ? 1 : 0);
    }
    else if (opcode == 0x6A) // ROR A
    {
        uint8_t oldC = status.c;
        status.c = A & 1;
        A = (A >> 1) | (oldC << 7);
        status.z = (A == 0 ? 1 : 0);
        status.n = (((A >> 7) & 1) == 1 ? 1 : 0);
    }
    else if(opcode == 0x0B || opcode == 0x2B){
        uint8_t m = read(address);   
        A = A & m;                   
        status.z = (A == 0 ? 1 : 0);
        status.n = ((A & 0x80) != 0 ? 1 : 0);
        status.c = status.n; 
    }
    else
    {
        (this->*(ins.operate))(address);
    }
    status.to_byte();
    cycles += ins.cycle;
    if (pageCrossed && checkpagecross.find(opcode) != checkpagecross.end() && !(opcode >= 0x10 && opcode <= 0x70 && (opcode & 0x1F) == 0x10))
    {
        cycles += 1;
    }
    totalcycles += cycles;
}

std::string CPU6502::debugStr()
{
    if (cycles != 0)
    {
        return "";
    }

    std::ostringstream os;
    std::uint8_t nextOpcode = read(PC);

    os << std::hex << std::uppercase << std::right << std::setfill('0');
    os << std::setw(4) << PC << "  " << std::setw(2) << +nextOpcode << " " << CPU6502::lookup[nextOpcode].opcodename << "         ";
    os << " A:" << std::setw(2) << +A
       << " X:" << std::setw(2) << +X
       << " Y:" << std::setw(2) << +Y
       << " P:" << std::setw(2) << +status.value
       << " SP:" << std::setw(2) << +SP
       << " CYC:" << std::dec << totalcycles;
    os << "\n";

    return os.str();
}

void CPU6502::ADC(uint16_t address)
{
    uint8_t m = read(address);
    uint16_t temp = uint16_t(A) + uint16_t(m) + uint16_t(status.c);
    status.c = (temp > 0xFF ? 1 : 0);
    status.v = (~(A ^ m) & (A ^ temp) & 0x80 ? 1 : 0);
    A = uint8_t(temp);
    status.z = (A == 0 ? 1 : 0);
    status.n = (A & 0x80 ? 1 : 0);
}


void CPU6502::AND(uint16_t address)
{
    A = A & read(address);
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::ASL(uint16_t address)
{
    uint8_t m = read(address);
    status.c = (m & 0x80) >> 7;
    m <<= 1;
    write(address, m);
    status.z = (m == 0 ? 1 : 0);
    status.n = ((m & 0x80) != 0 ? 1 : 0);
}

void CPU6502::BCC(uint16_t address)
{
    if (!status.c)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;

        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BCS(uint16_t address)
{
    if (status.c)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;
        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BEQ(uint16_t address)
{
    if (status.z)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;
        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BIT(uint16_t address)
{
    uint8_t m = read(address);
    status.z = ((m & A) == 0 ? 1 : 0);
    status.v = ((m & 0x40) != 0 ? 1 : 0);
    status.n = ((m & 0x80) != 0 ? 1 : 0);
}

void CPU6502::BMI(uint16_t address)
{
    if (status.n)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;

        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BNE(uint16_t address)
{
    if (!status.z)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;

        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BPL(uint16_t address)
{
    if (!status.n)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;

        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BRK(uint16_t address)
{
    push16(PC);
    status.i = 1;
    PHP(address);
    PC = read16(0xFFFE);
}
void CPU6502::BVC(uint16_t address)
{
    if (!status.v)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;

        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::BVS(uint16_t address)
{
    if (status.v)
    {
        uint16_t old_pc = PC;
        PC += static_cast<int8_t>(address);
        cycles += 1;

        if ((old_pc & 0xFF00) != (PC & 0xFF00))
        {
            cycles += 1;
        }
    }
}

void CPU6502::CLC(uint16_t)
{
    status.c = 0;
}

void CPU6502::CLD(uint16_t)
{
    status.d = 0;
}

void CPU6502::CLI(uint16_t)
{
    status.i = 0;
}

void CPU6502::CLV(uint16_t)
{
    status.v = 0;
}

void CPU6502::CMP(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t result = A - m;

    status.c = (A >= m ? 1 : 0);
    status.z = (A == m ? 1 : 0);
    status.n = ((result & 0x80) != 0 ? 1 : 0);
}

void CPU6502::CPX(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t result = X - m;

    status.c = (X >= m ? 1 : 0);
    status.z = (X == m ? 1 : 0);
    status.n = ((result & 0x80) != 0 ? 1 : 0);
}

void CPU6502::CPY(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t result = Y - m;

    status.c = (Y >= m ? 1 : 0);
    status.z = (Y == m ? 1 : 0);
    status.n = ((result & 0x80) != 0 ? 1 : 0);
}

void CPU6502::DEC(uint16_t address)
{
    uint8_t m = read(address);
    m--;
    write(address, m);

    status.z = (m == 0 ? 1 : 0);
    status.n = ((m & 0x80) != 0 ? 1 : 0);
}

void CPU6502::DEX(uint16_t)
{
    X--;
    status.z = (X == 0 ? 1 : 0);
    status.n = ((X & 0x80) != 0 ? 1 : 0);
}

void CPU6502::DEY(uint16_t)
{
    Y--;
    status.z = (Y == 0 ? 1 : 0);
    status.n = ((Y & 0x80) != 0 ? 1 : 0);
}

void CPU6502::EOR(uint16_t address)
{
    A = A ^ read(address);
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::INC(uint16_t address)
{
    uint8_t m = read(address);
    m++;
    write(address, m);

    status.z = (m == 0 ? 1 : 0);
    status.n = ((m & 0x80) != 0 ? 1 : 0);
}

void CPU6502::INX(uint16_t)
{
    X++;
    status.z = (X == 0 ? 1 : 0);
    status.n = ((X & 0x80) != 0 ? 1 : 0);
}

void CPU6502::INY(uint16_t)
{
    Y++;
    status.z = (Y == 0 ? 1 : 0);
    status.n = ((Y & 0x80) != 0 ? 1 : 0);
}

void CPU6502::JMP(uint16_t address)
{
    PC = address;
}

void CPU6502::JSR(uint16_t address)
{
    push16(PC - 1);
    PC = address;
}

void CPU6502::LDA(uint16_t address)
{
    A = read(address);
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::LDX(uint16_t address)
{
    X = read(address);
    status.z = (X == 0 ? 1 : 0);
    status.n = ((X & 0x80) != 0 ? 1 : 0);
}

void CPU6502::LDY(uint16_t address)
{
    Y = read(address);
    status.z = (Y == 0 ? 1 : 0);
    status.n = ((Y & 0x80) != 0 ? 1 : 0);
}

void CPU6502::LSR(uint16_t address)
{
    uint8_t m = read(address);
    status.c = m & 1;
    m >>= 1;
    write(address, m);
    status.z = (m == 0 ? 1 : 0);
    status.n = 0;
}

void CPU6502::NOP(uint16_t)
{
}

void CPU6502::ORA(uint16_t address)
{
    A = A | read(address);
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::PHA(uint16_t)
{
    push(A);
}

void CPU6502::PHP(uint16_t)
{

    push(status.value | 0x30);
}

void CPU6502::PLA(uint16_t)
{
    A = pop();
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::PLP(uint16_t)
{
    status.value = (pop() & 0xEF) | 0x20;
    status.from_byte(status.value);
}

void CPU6502::ROL(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t oldC = status.c;
    status.c = (m & 0x80) >> 7;
    m = (m << 1) | oldC;
    write(address, m);
    status.z = (m == 0 ? 1 : 0);
    status.n = ((m & 0x80) != 0 ? 1 : 0);
}

void CPU6502::ROR(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t oldC = status.c;
    status.c = m & 1;
    m = (m >> 1) | (oldC << 7);
    write(address, m);
    status.z = (m == 0 ? 1 : 0);
    status.n = ((m & 0x80) != 0 ? 1 : 0);
}

void CPU6502::RTI(uint16_t)
{

    status.value = (pop() & 0xEF) | 0x20;
    status.from_byte(status.value);
    PC = pop16();
}

void CPU6502::RTS(uint16_t)
{
    PC = pop16() + 1;
}

void CPU6502::SBC(uint16_t address)
{
    uint8_t m = read(address);
    uint16_t diff = uint16_t(A) - m - (1 - status.c);

    status.c = (diff <= 0xFF ? 1 : 0); // Carry clear on borrow
    status.v = ((A ^ diff) & (~m ^ diff) & 0x80 ? 1 : 0);

    A = uint8_t(diff);
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::SEC(uint16_t)
{
    status.c = 1;
}

void CPU6502::SED(uint16_t)
{
    status.d = 1;
}

void CPU6502::SEI(uint16_t)
{
    status.i = 1;
}

void CPU6502::STA(uint16_t address)
{
    write(address, A);
}

void CPU6502::STX(uint16_t address)
{
    write(address, X);
}

void CPU6502::STY(uint16_t address)
{
    write(address, Y);
}

void CPU6502::TAX(uint16_t)
{
    X = A;
    status.z = (X == 0 ? 1 : 0);
    status.n = ((X & 0x80) != 0 ? 1 : 0);
}

void CPU6502::TAY(uint16_t)
{
    Y = A;
    status.z = (Y == 0 ? 1 : 0);
    status.n = ((Y & 0x80) != 0 ? 1 : 0);
}

void CPU6502::TSX(uint16_t)
{
    X = SP;
    status.z = (X == 0 ? 1 : 0);
    status.n = ((X & 0x80) != 0 ? 1 : 0);
}

void CPU6502::TXA(uint16_t)
{
    A = X;
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::TXS(uint16_t)
{
    SP = X;
}

void CPU6502::TYA(uint16_t)
{
    A = Y;
    status.z = (A == 0 ? 1 : 0);
    status.n = ((A & 0x80) != 0 ? 1 : 0);
}

void CPU6502::XXX(uint16_t)
{
}

//Undocumented Opcodes

void CPU6502::AHX(uint16_t address) {
    
    uint8_t value = A & X & ((address >> 8) + 1);
    write(address, value);
}

void CPU6502::SHX(uint16_t address) {
    uint8_t value = X & ((address >> 8) + 1);
    write(address, value);
}

void CPU6502::SAX(uint16_t address) {
    uint8_t value = A & X;
    bus->CPUwrite(address, value);
}

void CPU6502::KIL(uint16_t /*address*/) {
   //Unimplemented
}

void CPU6502::TAS(uint16_t address) {
    SP = A & X;
    uint8_t value = A & X & ((address >> 8) + 1);
    write(address, value);
}

void CPU6502::LAS(uint16_t address) {
    uint8_t value = read(address) & SP;
    A = X = SP = value;
    status.z = (value == 0 ? 1 : 0);
    status.n = ((value & 0x80) != 0 ? 1 : 0);
}

void CPU6502::AXS(uint16_t address) {
    uint8_t m = read(address);
    uint8_t temp = (A & X) - m;   
    status.c = ((A & X) >= m) ? 1 : 0;
    X = temp;
    status.z = (X == 0 ? 1 : 0);
    status.n = ((X & 0x80) != 0 ? 1 : 0);
}

void CPU6502::SLO(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t result = m << 1;
    status.c = (m & 0x80) != 0;
    write(address, result);
    A |= result;
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::RLA(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t result = (m << 1) | status.c;
    status.c = (m & 0x80) != 0;
    write(address, result);
    A &= result;
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::SRE(uint16_t address)
{
    uint8_t m = read(address);
    status.c = (m & 0x01) != 0;
    uint8_t result = m >> 1;
    write(address, result);
    A ^= result;
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::ALR(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t anded = A & m;
    status.c = (anded & 0x01) != 0;
    A = anded >> 1;
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::RRA(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t rotated = (m >> 1) | (status.c << 7);
    status.c = (m & 0x01) != 0;
    write(address, rotated);

    uint16_t temp = uint16_t(A) + uint16_t(rotated) + uint16_t(status.c);
    status.c = (temp > 0xFF);
    status.v = (~(A ^ rotated) & (A ^ temp) & 0x80) != 0;
    A = uint8_t(temp);
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::ARR(uint16_t address)
{
    uint8_t m = read(address);
    A &= m;
    uint8_t oldA = A;
    A = (A >> 1) | (status.c << 7);
    status.c = (A & 0x40) >> 6;               
    status.v = ((A >> 5) ^ (A >> 6)) & 1;     
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::XAA(uint16_t address)
{
    uint8_t m = read(address);
    uint8_t temp = X & m;
    if (temp < m)
        temp &= 0xEF;  
    if (temp & 0x01 == 0)
        temp &= 0xFE;  
    A = temp;
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}



void CPU6502::LAX(uint16_t address)
{
    uint8_t m = read(address);
    A = m;
    X = m;
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::DCP(uint16_t address)
{
    uint8_t m = read(address) - 1;
    write(address, m);

    uint16_t temp = uint16_t(A) - uint16_t(m);
    status.c = (A >= m);
    status.z = (temp == 0);
    status.n = (temp & 0x80) != 0;
}

void CPU6502::ISC(uint16_t address)
{
    uint8_t m = read(address) + 1;
    write(address, m);

    uint16_t temp = uint16_t(A) - uint16_t(m) - (1 - status.c);
    status.c = (A >= (m + (1 - status.c)));
    status.v = ((A ^ temp) & (~m ^ temp) & 0x80) != 0;
    A = uint8_t(temp);
    status.z = (A == 0);
    status.n = (A & 0x80) != 0;
}

void CPU6502::ANC(uint16_t address){
    // Already Implemented        
}