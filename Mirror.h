#pragma once

// NES nametable mirroring modes
enum class MIRROR
{
    HORIZONTAL,     // iNES: 0
    VERTICAL,       // iNES: 1
    ONE_SCREEN_LO,  // $2000
    ONE_SCREEN_HI,  // $2400
    FOUR_SCREEN,    // 4-screen VRAM
    HARDWARE        // use iNES header (no mapper override)
};
