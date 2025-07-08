#include "Screen.h"

#include "Core/Logger.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/IoRegisters.h"

Screen::Screen() : _window(nullptr), _renderer(nullptr), _shouldClose(false)
{
}

Screen::~Screen()
{
    Shutdown();
}

bool Screen::Initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        DEBUGBREAKLOG("SDL could not initialize! SDL_Error: " << SDL_GetError());
        return false;
    }

    _window = SDL_CreateWindow("OGBEmu",
        SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
        0);

    if (_window == nullptr)
    {
        DEBUGBREAKLOG("Window could not be created! SDL_Error: " << SDL_GetError());
        SDL_Quit();
        return false;
    }

    _renderer = SDL_CreateRenderer(_window, nullptr);
    if (_renderer == nullptr)
    {
        DEBUGBREAKLOG("Renderer could not be created! SDL_Error: " << SDL_GetError());
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }

    LOG("Screen initialized successfully");
    return true;
}

void Screen::Shutdown()
{
    if (_renderer)
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }

    if (_window)
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    SDL_Quit();
    LOG("Screen shutdown completed");
}

void Screen::Clear()
{
    SDL_SetRenderDrawColor(_renderer, 155, 188, 15, 255); // Game Boy green
    SDL_RenderClear(_renderer);
}

void Screen::Present()
{
    SDL_RenderPresent(_renderer);
    
    // Handle SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            _shouldClose = true;
        }
    }
}

bool Screen::ShouldClose() const
{
    return _shouldClose;
}

void Screen::RenderBackground(const VRam* vram, const IoRegisters* ioRegisters)
{
    // Background tile map starts at 0x9800 (can also be 0x9C00 based on LCDC)
    constexpr int BG_MAP_START = 0x9800;
    
    // Render 20x18 tiles (visible area is 160x144 pixels)
    for (int tileY = 0; tileY < 18; ++tileY)
    {
        for (int tileX = 0; tileX < 20; ++tileX)
        {
            // Get tile index from background map
            const int mapIndex = (tileY * BACKGROUND_WIDTH) + tileX;
            const int tileIndex = vram->Read(BG_MAP_START + mapIndex);
            
            // Render the tile
            RenderTile(tileIndex, tileX * TILE_SIZE, tileY * TILE_SIZE, vram);
        }
    }
}

void Screen::RenderTile(int tileIndex, int x, int y, const VRam* vram)
{
    // Tile data starts at 0x8000
    constexpr int TILE_DATA_START = 0x8000;
    const int tileDataAddress = TILE_DATA_START + (tileIndex * 16); // 16 bytes per tile
    
    // Each tile is 8x8 pixels, 2 bits per pixel
    for (int row = 0; row < TILE_SIZE; ++row)
    {
        // Each row is 2 bytes (low and high bits)
        const byte lowByte = vram->Read(tileDataAddress + (row * 2));
        const byte highByte = vram->Read(tileDataAddress + (row * 2) + 1);
        
        for (int col = 0; col < TILE_SIZE; ++col)
        {
            // Get 2-bit color value for this pixel
            const int bit = 7 - col;
            const int colorIndex = ((highByte >> bit) & 1) << 1 | ((lowByte >> bit) & 1);
            
            // Set pixel color
            const SDL_Color& color = PALETTE[colorIndex];
            SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
            
            // Draw pixel (scaled up by WINDOW_SCALE)
            SDL_FRect rect = {
                static_cast<float>((x + col) * WINDOW_SCALE),
                static_cast<float>((y + row) * WINDOW_SCALE),
                static_cast<float>(WINDOW_SCALE),
                static_cast<float>(WINDOW_SCALE)
            };
            SDL_RenderFillRect(_renderer, &rect);
        }
    }
}