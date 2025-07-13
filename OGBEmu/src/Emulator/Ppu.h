#pragma once

#include "Core/Definitions.h"

class Bus;
class IScreen;

class Ppu
{
public:
    Ppu(Bus* bus, IScreen* screen);

    void Update(int cycles);
    void RenderFrame();
    
    // PPU timing constants
    static constexpr int CYCLES_PER_SCANLINE = 456;
    static constexpr int VISIBLE_SCANLINES = 144;
    static constexpr int VBLANK_SCANLINES = 10;
    static constexpr int TOTAL_SCANLINES = VISIBLE_SCANLINES + VBLANK_SCANLINES;
    
    // PPU mode timing constants (cycles within a scanline)
    static constexpr int OAM_SEARCH_CYCLES = 80;   // Mode 2
    static constexpr int DRAWING_CYCLES = 172;     // Mode 3
    static constexpr int HBLANK_CYCLES = 204;      // Mode 0
    
    // PPU modes
    enum class PpuMode : byte
    {
        HBlank = 0,     // Mode 0: H-Blank
        VBlank = 1,     // Mode 1: V-Blank  
        OamSearch = 2,  // Mode 2: OAM Search
        Drawing = 3     // Mode 3: Drawing pixels
    };

private:
    void RenderBackground();
    void RenderSprites();
    void RenderWindow();
    [[nodiscard]] byte GetTilePixel(word tileIndex, byte pixelX, byte pixelY) const;
    [[nodiscard]] byte GetSpritePixel(word tileIndex, byte pixelX, byte pixelY) const;
    [[nodiscard]] word GetBackgroundPalette() const;
    void SetPixel(int x, int y, byte colorIndex);
    
    // PPU mode and STAT register management
    void UpdatePpuMode();
    void UpdateStatRegister();
    void CheckStatInterrupts();
    [[nodiscard]] bool IsLyLycMatch() const;

    Bus* _bus;
    IScreen* _screen;

    // Game Boy screen dimensions
    static constexpr int SCREEN_WIDTH = 160;
    static constexpr int SCREEN_HEIGHT = 144;
    
    // Tile dimensions
    static constexpr int TILE_SIZE = 8;
    static constexpr int TILES_PER_ROW = 32;
    static constexpr int TILES_PER_COL = 32;
    
    // Memory addresses
    static constexpr word TILE_DATA_START = 0x8000;
    static constexpr word BACKGROUND_MAP_START = 0x9800;
    
    // Frame buffer for rendered pixels
    byte _frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    
    // PPU timing state
    int _currentCycles;
    byte _currentScanline;
    PpuMode _currentMode;
};