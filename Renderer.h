#pragma once
#include <SDL2/SDL.h>
#include "PPU2C02.h"

class Renderer {
public:
    static constexpr int WIDTH = 256;
    static constexpr int HEIGHT = 240;

    Renderer();
    ~Renderer();

    void drawFrame(PPU2C02& ppu);
    void drawOAMDebug(uint32_t* pixels, int w, int h);
    void beginFrame();
    void presentFrame();


private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* oamTexture = nullptr;
    SDL_Texture* texture = nullptr;
};
