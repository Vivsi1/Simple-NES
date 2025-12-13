#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <memory>

class Bus;

class Cartridge;

class PPU2C02 {
public:
    Bus* bus = nullptr;
    PPU2C02(); 
    void connectBus(Bus* bus);
    void connectCartridge(const std::shared_ptr<Cartridge>& c) { cart = c; }

    void    CPUwrite(uint16_t addr, uint8_t data);
    uint8_t CPUread(uint16_t addr);

    uint8_t PPUread(uint16_t addr);
    void    PPUwrite(uint16_t addr, uint8_t data);

    mutable std::vector<uint8_t> vram = std::vector<uint8_t>(2048); //2 x (Nametable(32*30 = 960 bytes) + Attribute Table(64 bytes)) = 2048. Info read from pattern table in cartridge   
    std::array<uint8_t, 0x20>  palette{}; //Frame Palette is eight groups of colors of four colors each.
    std::array<uint8_t, 256> primaryoam{};  //We have to work on this now. Seems like refreshed every frame and contains 64 sprites. Contans 4 things for a sprite(Byte 0: Y position (minus 1), Byte 1: Tile index, Byte 2: Attributes (palette, flipping, priority) ,Byte 3: X position)
    //Apart from that I believe only 8 sprites per frame loaded/evaluated from the SMB3 video by Bismuth. There are two oams in the system. The primary one listed above and the secondary one
    struct SecondaryOAM {
        std::array<uint8_t, 32> data {};   // 8 sprites × 4 bytes
        void clear() { data.fill(0xFF); }
    } secondary_oam;
    std::array<uint8_t, 8> secondary_oam_index{}; // track original OAM index for each secondaryOAM slot (0..7), 0xFF = empty


    struct Color {
    uint8_t r, g, b;
    };

    std::array<Color, 64> systempalette{
    Color{0x7C, 0x7C, 0x7C}, Color{0x00, 0x00, 0xFC}, Color{0x00, 0x00, 0xBC}, Color{0x44, 0x28, 0xBC},
    Color{0x94, 0x00, 0x84}, Color{0xA8, 0x00, 0x20}, Color{0xA8, 0x10, 0x00}, Color{0x88, 0x14, 0x00},
    Color{0x50, 0x30, 0x00}, Color{0x00, 0x78, 0x00}, Color{0x00, 0x68, 0x00}, Color{0x00, 0x58, 0x00},
    Color{0x00, 0x40, 0x58}, Color{0x00, 0x00, 0x00}, Color{0x00, 0x00, 0x00}, Color{0x00, 0x00, 0x00},
    Color{0xBC, 0xBC, 0xBC}, Color{0x00, 0x78, 0xF8}, Color{0x00, 0x58, 0xF8}, Color{0x68, 0x44, 0xFC},
    Color{0xD8, 0x00, 0xCC}, Color{0xE4, 0x00, 0x58}, Color{0xF8, 0x38, 0x00}, Color{0xE4, 0x5C, 0x10},
    Color{0xAC, 0x7C, 0x00}, Color{0x00, 0xB8, 0x00}, Color{0x00, 0xA8, 0x00}, Color{0x00, 0xA8, 0x44},
    Color{0x00, 0x88, 0x88}, Color{0x00, 0x00, 0x00}, Color{0x00, 0x00, 0x00}, Color{0x00, 0x00, 0x00},
    Color{0xF8, 0xF8, 0xF8}, Color{0x3C, 0xBC, 0xFC}, Color{0x68, 0x88, 0xFC}, Color{0x98, 0x78, 0xF8},
    Color{0xF8, 0x78, 0xF8}, Color{0xF8, 0x58, 0x98}, Color{0xF8, 0x78, 0x58}, Color{0xFC, 0xA0, 0x44},
    Color{0xF8, 0xB8, 0x00}, Color{0xB8, 0xF8, 0x18}, Color{0x58, 0xD8, 0x54}, Color{0x58, 0xF8, 0x98},
    Color{0x00, 0xE8, 0xD8}, Color{0x78, 0x78, 0x78}, Color{0x00, 0x00, 0x00}, Color{0x00, 0x00, 0x00},
    Color{0xFC, 0xFC, 0xFC}, Color{0xA4, 0xE4, 0xFC}, Color{0xB8, 0xB8, 0xF8}, Color{0xD8, 0xB8, 0xF8},
    Color{0xF8, 0xB8, 0xF8}, Color{0xF8, 0xA4, 0xC0}, Color{0xF0, 0xD0, 0xB0}, Color{0xFC, 0xE0, 0xA8},
    Color{0xF8, 0xD8, 0x78}, Color{0xD8, 0xF8, 0x78}, Color{0xB8, 0xF8, 0xB8}, Color{0xB8, 0xF8, 0xD8},
    Color{0x00, 0xFC, 0xFC}, Color{0xF8, 0xD8, 0xF8}, Color{0x00, 0x00, 0x00}, Color{0x00, 0x00, 0x00}
    }; //Initialise NES system palette
    std::array<std::array<Color, 256>, 240> framebuffer; //Store frame as array of pixels
   
    int16_t scanline_cycle = 0; // -1 pre-render, 0-239 visible, 240 post, 241-260 vblank
    int16_t dot = 0; // 0-340
    bool frame_complete = false; //Measure frame completion
    bool oddFrame = false;
    bool nmiOccurred = false;

    //PPU Registers
    struct PPUCTRL {
        uint8_t value;
        uint8_t nametableX   : 1; 
        uint8_t nametableY   : 1; 
        uint8_t increment    : 1; 
        uint8_t spriteTbl    : 1; 
        uint8_t bgTbl        : 1; 
        uint8_t spriteSize   : 1; 
        uint8_t masterSlave  : 1; 
        uint8_t nmiEnable    : 1;

        PPUCTRL(uint8_t val = 0) { from_byte(val); }

        void from_byte(uint8_t val) {
            value       = val;
            nametableX  = val & 0x01;
            nametableY  = (val >> 1) & 0x01;
            increment   = (val >> 2) & 0x01;
            spriteTbl   = (val >> 3) & 0x01;
            bgTbl       = (val >> 4) & 0x01;
            spriteSize  = (val >> 5) & 0x01;
            masterSlave = (val >> 6) & 0x01;
            nmiEnable   = (val >> 7) & 0x01;
        }

        uint8_t to_byte() {
            value = (nametableX) |
                    (nametableY << 1) |
                    (increment  << 2) |
                    (spriteTbl  << 3) |
                    (bgTbl      << 4) |
                    (spriteSize << 5) |
                    (masterSlave<< 6) |
                    (nmiEnable  << 7);
            return value;
        }
    };
    PPUCTRL ppuctrl{0x00};

    struct PPUMASK {
        uint8_t value;
        uint8_t greyscale       : 1;
        uint8_t showLeftBG      : 1;
        uint8_t showLeftSprites : 1;
        uint8_t showBG          : 1;
        uint8_t showSprites     : 1;
        uint8_t emphasizeRed    : 1;
        uint8_t emphasizeGreen  : 1;
        uint8_t emphasizeBlue   : 1;

        PPUMASK(uint8_t val = 0) { from_byte(val); }

        void from_byte(uint8_t val) {
            value          = val;
            greyscale      = val & 0x01;
            showLeftBG     = (val >> 1) & 0x01;
            showLeftSprites= (val >> 2) & 0x01;
            showBG         = (val >> 3) & 0x01;
            showSprites    = (val >> 4) & 0x01;
            emphasizeRed   = (val >> 5) & 0x01;
            emphasizeGreen = (val >> 6) & 0x01;
            emphasizeBlue  = (val >> 7) & 0x01;
        }

        uint8_t to_byte() {
            value = (greyscale) |
                    (showLeftBG << 1) |
                    (showLeftSprites << 2) |
                    (showBG << 3) |
                    (showSprites << 4) |
                    (emphasizeRed << 5) |
                    (emphasizeGreen << 6) |
                    (emphasizeBlue << 7);
            return value;
        }
    };
    PPUMASK ppumask{0x00};


    struct PPUSTATUS {
        uint8_t value;
        uint8_t unused    : 5; // usually 0
        uint8_t spriteOverflow : 1;
        uint8_t spriteZeroHit : 1;
        uint8_t vblank        : 1;

        PPUSTATUS(uint8_t val = 0) { from_byte(val); }

        void from_byte(uint8_t val) {
            value          = val;
            unused         = val & 0x1F;
            spriteOverflow = (val >> 5) & 0x01;
            spriteZeroHit  = (val >> 6) & 0x01;
            vblank         = (val >> 7) & 0x01;
        }

        uint8_t to_byte() {
            value = (unused) |
                    (spriteOverflow << 5) |
                    (spriteZeroHit  << 6) |
                    (vblank         << 7);
            return value;
        }
    };
    PPUSTATUS ppustatus{0x00};
    uint8_t  oamaddr{0};   
    uint8_t  readBuffer{};

    // Internal registers
    uint16_t v{}; //During rendering, used for the scroll position. Outside of rendering, used as the current VRAM address. 
    uint16_t t{}; //During rendering, specifies the starting coarse-x scroll for the next scanline and the starting y scroll for the screen. Outside of rendering, holds the scroll or VRAM address before transferring it to v.  
    uint8_t  x{}; //The fine-x position of the current scroll, used during rendering alongside v  
    bool     w{}; //Toggles on each write to either PPUSCROLL or PPUADDR, indicating whether this is the first or second write. Clears on reads of PPUSTATUS. Sometimes called the 'write latch' or 'write toggle'.

    struct TileFetch {
        uint8_t nt = 0;   // nametable byte
        uint8_t at = 0;   // attribute byte (0–3)
        uint8_t lo = 0;   // pattern low
        uint8_t hi = 0;   // pattern high
    };
    TileFetch bg_latch; // used by dots 1-256 and 321-336 to place fetched data to

    struct BGShifters {
        uint16_t pattern_lo = 0;
        uint16_t pattern_hi = 0;
        uint16_t attrib_lo  = 0;
        uint16_t attrib_hi  = 0;
    };
    BGShifters bg_shift; // Shifted by 1 every dot. Top half reloaded every dot % 8 + 1 time

    struct SpriteEval {
        int n = 0;          // sprite index (0..63)
        int m = 0;          // byte index within sprite (0..3)
        uint8_t latch = 0;  // odd-cycle latch
        int found = 0;      // number of sprites copied
        bool writesDisabled = false;
        int copy = 0;
        int cycleGuard = 0; // safety
    } sprite_eval;

    struct SpriteFetchEntry {
        uint8_t y = 0xFF;
        uint8_t tile = 0xFF;
        uint8_t attr = 0xFF;
        uint8_t x = 0xFF;
        bool valid = false;
        bool isSpriteZero = false;
    };
    std::array<SpriteFetchEntry, 8> sprite_fetch;

    struct SpriteShifter {
        uint16_t lo = 0;
        uint16_t hi = 0;
        int x_counter = 0;
        uint8_t palette = 0;
        uint8_t priority = 0;
        bool valid = false;
        bool isSpriteZero = false;
    };
    std::array<SpriteShifter, 8> sprite_shifters;

    bool spriteZeroInLine = false;
    uint8_t openBus = 0;
    std::shared_ptr<Cartridge> cart;
    uint16_t mapNametableAddr(uint16_t addr) const; // apply mirroring
    uint16_t incAmount();
    void tick();
    void debugOAMToTexture(uint32_t* out, int texW, int texH);
    void decodeTileToBuffer(uint8_t tile, uint8_t paletteIndex, uint32_t* outPixels);
    void render_scanline();
    void drawPixel(int x, int y, uint8_t palette, uint8_t pixel);
    void shiftBGShifters();
    void loadBGShifters();
    void incrementScrollX();
    void incrementScrollY();
    void spriteEvaluation(int dot);
    void fetchSpriteTile(int dot);
    void shiftSpriteShifters();
    void getSpritePixel(uint8_t x, uint8_t &pixel, uint8_t &palette, bool &priority, bool &isSpriteZero);
};
