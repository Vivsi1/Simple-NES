#include "Renderer.h"
#include <SDL2/SDL.h>
#include <iostream>

Renderer::Renderer() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << "\n";
        return;
    }

    window = SDL_CreateWindow(
        "NES Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH * 3,
        HEIGHT * 3,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        return;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    if (!texture) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << "\n";
        return;
    }
}

Renderer::~Renderer() {
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void Renderer::drawFrame(const PPU2C02& ppu) {

    void* pixels = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
        std::cerr << "LockTexture failed: " << SDL_GetError() << "\n";
        return;
    }
    
    uint32_t* dst = reinterpret_cast<uint32_t*>(pixels);

    for (int y = 0; y < HEIGHT; y++) {
        uint32_t* row = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uint8_t*>(dst) + y * pitch
        );

        for (int x = 0; x < WIDTH; x++) {
            auto c = ppu.framebuffer[y][x];
            row[x] = 0xFF000000 | (c.r << 16) | (c.g << 8) | c.b;
        }
    }

    SDL_UnlockTexture(texture);
    
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);  // SDL2 correct call
    SDL_RenderPresent(renderer);
}
