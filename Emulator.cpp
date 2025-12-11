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
        do {
            bus.cpu.clock();
            bus.ppu.tick();
            bus.ppu.tick();
            bus.ppu.tick();
        }
        while (!bus.ppu.frame_complete);

        // Present frame
        presentFrame();
        bus.ppu.frame_complete = false;

        // Now handle SDL input (once per frame)
        handleEvents();

        // FPS counter
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() >= 1000)
        {
            std::cout << "FPS: " << frames << "\n";
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
        {
            stop();
        }

        // Add controller / keyboard handling here:
        // e.g. when key pressed -> write to controller register on bus
        //
        // Example (pseudo):
        // if (event.type == SDL_EVENT_KEY_DOWN) { if (event.key.keysym.sym == SDLK_ESCAPE) stop(); }
    }
}

void Emulator::presentFrame()
{
    // This assumes your PPU provides a framebuffer that renderer.drawFrame accepts.
    // Earlier we used: ppu.framebuffer[y][x] returning Color { r,g,b }.
    renderer.drawFrame(bus.ppu);
}
