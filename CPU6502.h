#pragma once
#include <string>
#include <vector>
#include <cstdint>
using namespace std;


class CPU6502
{
public:
    using InstrFn = void (CPU6502::*)(uint16_t);

    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t SP;
    uint8_t F;
    uint16_t PC;

    enum class Addressing
    {
        IMP,
        ACC,
        IMM,
        ZP0,
        ZPX,
        ZPY,
        REL,
        ABS,
        ABX,
        ABY,
        IND,
        IZX,
        IZY,
    };

    struct instruction
    {
        string opcodename;
        InstrFn operate;
        Addressing addressing;
        int cycle;

        instruction(string name, InstrFn op, Addressing addr, int cyc)
            : opcodename(name), operate(op), addressing(addr), cycle(cyc) {}
    };

    uint8_t read(uint16_t addr);
    uint16_t read16(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
    void push(uint8_t data);
    void push16(uint16_t data);
    uint8_t pop();
    uint16_t pop16();

    void connectBus(Bus* bus);
    void clock();
    void reset();
    void nmi();
    void irq();

    void serialize(std::ostream &os) const;
    void deserialize(std::istream &is);
    bool isCrossed(uint16_t a, int16_t b);
    void setPc(std::uint16_t newPc);
    void execute();
    std::string debugStr();

    struct Status
    {
        uint8_t value;
        uint8_t c : 1; // Carry
        uint8_t z : 1; // Zero
        uint8_t i : 1; // Interrupt Disable
        uint8_t d : 1; // Decimal Mode
        uint8_t B : 1; // Break
        uint8_t u : 1; // Unused (usually 1)
        uint8_t v : 1; // Overflow
        uint8_t n : 1; // Negative

        void from_byte(uint8_t val)
        {
            c = val & 0x01;
            z = (val >> 1) & 0x01;
            i = (val >> 2) & 0x01;
            d = (val >> 3) & 0x01;
            B = (val >> 4) & 0x01;
            u = (val >> 5) & 0x01;
            v = (val >> 6) & 0x01;
            n = (val >> 7) & 0x01;
        }

        // Combine flags into byte
        void to_byte()
        {
            value = (c) |
                    (z << 1) |
                    (i << 2) |
                    (d << 3) |
                    (B << 4) |
                    (u << 5) |
                    (v << 6) |
                    (n << 7);
        }
    };

    vector<instruction> lookup;
    CPU6502();

    void ADC(uint16_t address);
    void AND(uint16_t address);
    void ASL(uint16_t address);
    void BCC(uint16_t address);
    void BCS(uint16_t address);
    void BEQ(uint16_t address);
    void BIT(uint16_t address);
    void BMI(uint16_t address);
    void BNE(uint16_t address);
    void BPL(uint16_t address);
    void BRK(uint16_t address);
    void BVC(uint16_t address);
    void BVS(uint16_t address);
    void CLC(uint16_t address);
    void CLD(uint16_t address);
    void CLI(uint16_t address);
    void CLV(uint16_t address);
    void CMP(uint16_t address);
    void CPX(uint16_t address);
    void CPY(uint16_t address);
    void DEC(uint16_t address);
    void DEX(uint16_t address);
    void DEY(uint16_t address);
    void EOR(uint16_t address);
    void INC(uint16_t address);
    void INX(uint16_t address);
    void INY(uint16_t address);
    void JMP(uint16_t address);
    void JSR(uint16_t address);
    void LDA(uint16_t address);
    void LDX(uint16_t address);
    void LDY(uint16_t address);
    void LSR(uint16_t address);
    void NOP(uint16_t address);
    void ORA(uint16_t address);
    void PHA(uint16_t address);
    void PHP(uint16_t address);
    void PLA(uint16_t address);
    void PLP(uint16_t address);
    void ROL(uint16_t address);
    void ROR(uint16_t address);
    void RTI(uint16_t address);
    void RTS(uint16_t address);
    void SBC(uint16_t address);
    void SEC(uint16_t address);
    void SED(uint16_t address);
    void SEI(uint16_t address);
    void STA(uint16_t address);
    void STX(uint16_t address);
    void STY(uint16_t address);
    void TAX(uint16_t address);
    void TAY(uint16_t address);
    void TSX(uint16_t address);
    void TXA(uint16_t address);
    void TXS(uint16_t address);
    void TYA(uint16_t address);
    void XXX(uint16_t address);
    void AHX(uint16_t address);
    void SHX(uint16_t address);
    void SAX(uint16_t address);
    void KIL(uint16_t address);
    void TAS(uint16_t address);
    void LAS(uint16_t address);
    void AXS(uint16_t address);
    void SLO(uint16_t address);
    void ANC(uint16_t address);
    void RLA(uint16_t address);
    void SRE(uint16_t address);
    void ALR(uint16_t address);
    void RRA(uint16_t address);
    void ARR(uint16_t address);
    void XAA(uint16_t address);
    void LAX(uint16_t address);
    void DCP(uint16_t address);
    void ISC(uint16_t address);

    Bus* bus = nullptr;
    Status status{status.value = 0x24};
    uint8_t cycles = 8;
    int totalcycles = 8;
};
