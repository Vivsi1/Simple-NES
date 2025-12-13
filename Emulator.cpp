#include "Emulator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <SDL2/SDL.h> // for event polling constants if needed

Emulator::Emulator()
{
    // Nothing fancy here — renderer ctor handles SDL init.
}

Emulator::~Emulator()
{
    stop();
}

bool Emulator::loadROM(const std::string &path)
{
    cart = std::make_shared<Cartridge>(path);
    if (!cart->isImageValid())
    {
        std::cerr << "Emulator: failed to load ROM: " << path << std::endl;
        return false;
    }

    // Insert cartridge into bus (assumes Bus::insertCartridge exists)
    bus.insertCartridge(cart);
    bus.cpu.connectBus(&bus);

    // Reset CPU and PPU states
    bus.cpu.reset();
    bus.ppu.connectBus(&bus);
    // Initialize PPU internal counters
    bus.ppu.scanline_cycle = -1; // pre-render line
    bus.ppu.dot = 0;
    bus.ppu.frame_complete = false;
    // Connect PPU to bus and cartridge (some of these calls may already be performed
    // inside Bus::insertCartridge; safe to repeat if your methods are idempotent)
    bus.ppu.connectBus(&bus);
    bus.ppu.connectCartridge(cart);

    return true;
}

void Emulator::reset()
{
    // Connect CPU to bus
    bus.cpu.connectBus(&bus);
    // Reset CPU and PPU states
    bus.cpu.reset();
    bus.ppu.connectBus(&bus);
    // Initialize PPU internal counters
    bus.ppu.scanline_cycle = -1; // pre-render line
    bus.ppu.dot = 0;
    bus.ppu.frame_complete = false;
    bus.ppu.v = bus.ppu.t = bus.ppu.x = bus.ppu.w = 0;
    // Reset cartridge / mapper state if you have such APIs (optional)
    // if (cart) cart->reset();
}

void Emulator::run()
{
    running = true;

    auto lastTime = std::chrono::high_resolution_clock::now();
    int frames = 0;

    while (running)
    {
        // Run cycles until a frame is produced
        handleEvents();
        do
        {
            bus.cpu.clock();
            bus.ppu.tick();
            bus.ppu.tick();
            bus.ppu.tick();
        } while (!bus.ppu.frame_complete);
        // Present frame
        presentFrame();
        bus.ppu.frame_complete = false;

        // Now handle SDL input (once per frame)

        // FPS counter
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() >= 1000)
        {
            frames = 0;
            lastTime = now;
        }
        frames++;

        // Optional throttling
        if (throttle)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
}

void Emulator::stop()
{
    running = false;
}

void Emulator::stepCycle()
{
    // Execute one CPU cycle. Your CPU implementation may produce either
    // one internal clock step (clock()) or require more. Example earlier used:
    // cpu.clock(); ppu.tick(); ppu.tick(); ppu.tick();
    //
    // We'll follow that approach.

    bus.cpu.clock();

    // PPU runs 3 ticks per CPU cycle
    bus.ppu.tick();
    bus.ppu.tick();
    bus.ppu.tick();
}

void Emulator::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            stop();
    }

    // ---- CONTROLLER STATE POLLING ----
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    bus.setButton(0, Bus::NES_A,      keys[SDL_SCANCODE_Z]);
    bus.setButton(0, Bus::NES_B,      keys[SDL_SCANCODE_X]);
    bus.setButton(0, Bus::NES_SELECT, keys[SDL_SCANCODE_RSHIFT]);
    bus.setButton(0, Bus::NES_START,  keys[SDL_SCANCODE_RETURN]);
    bus.setButton(0, Bus::NES_UP,     keys[SDL_SCANCODE_UP]);
    bus.setButton(0, Bus::NES_DOWN,   keys[SDL_SCANCODE_DOWN]);
    bus.setButton(0, Bus::NES_LEFT,   keys[SDL_SCANCODE_LEFT]);
    bus.setButton(0, Bus::NES_RIGHT,  keys[SDL_SCANCODE_RIGHT]);
}

void Emulator::presentFrame()
{
    // 1. Clear and draw NES framebuffer
    renderer.beginFrame();       // SDL_RenderClear
    renderer.drawFrame(bus.ppu); // SDL_RenderCopy(texture)

    // 2. Prepare OAM debug buffer
    // static uint32_t oamDebugTex[64 * 64];
    // bus.ppu.debugOAMToTexture(oamDebugTex, 64, 64);

    // 3. Draw the OAM Debug overlay ON TOP
    // renderer.drawOAMDebug(oamDebugTex, 64, 64);

    // 4. PRESENT — MUST BE HERE
    renderer.presentFrame(); // SDL_RenderPresent()
}
