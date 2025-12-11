#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "Bus.h"        // your Bus (contains CPU & PPU instances)
#include "Cartridge.h"  // your Cartridge
#include "Renderer.h"   // SDL3 renderer from earlier

class Emulator {
public:
    Emulator();
    ~Emulator();

    // Load ROM file. Returns true on success.
    bool loadROM(const std::string& path);

    // Reset CPU / PPU and prepare to run.
    void reset();

    // Run until the window is closed (blocking).
    void run();

    // Stop the emulator loop (safe to call from another thread).
    void stop();

private:
    // Single-step: one CPU cycle + 3 PPU ticks (typical NES timing)
    void stepCycle();

    // Poll platform events (window close, inputs)
    void handleEvents();

    // Helper to present framebuffer when PPU signals frame complete
    void presentFrame();

    // Members
    Bus bus; // bus contains cpu and ppu objects (matching your code)
    std::shared_ptr<Cartridge> cart;
    Renderer renderer;

    std::atomic<bool> running{false};

    // Frame timing / optional throttle (not implemented precisely)
    bool throttle = false; // set true to throttle to ~60Hz (basic)
};
