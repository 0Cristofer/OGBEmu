#pragma once

#include <SDL3/SDL.h>
#include "Core/Definitions.h"
#include "IScreen.h"

class Screen : public IScreen
{
public:
    Screen();
    ~Screen();

    bool Initialize() override;
    void Shutdown() override;
    void Clear() override;
    void Present() override;
    bool ShouldClose() const override;
    
    void RenderBackground(const VRam* vram, const IoRegisters* ioRegisters) override;

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    bool _shouldClose;

    static constexpr int SCREEN_WIDTH = 160;
    static constexpr int SCREEN_HEIGHT = 144;
    static constexpr int WINDOW_SCALE = 4;
    
    // Game Boy tile constants
    static constexpr int TILE_SIZE = 8;
    static constexpr int BACKGROUND_WIDTH = 32;
    static constexpr int BACKGROUND_HEIGHT = 32;
    
    // Game Boy color palette (grayscale)
    static constexpr SDL_Color PALETTE[4] = {
        {224, 248, 208, 255}, // Lightest
        {136, 192, 112, 255}, // Light
        {52, 104, 86, 255},   // Dark
        {8, 24, 32, 255}      // Darkest
    };
    
    void RenderTile(int tileIndex, int x, int y, const class VRam* vram, byte palette);
};