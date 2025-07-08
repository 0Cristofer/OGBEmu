#pragma once

#include "Core/Definitions.h"

class VRam;
class Oam;
class IoRegisters;
class Screen;

class Ppu
{
public:
    Ppu(VRam* vRam, Oam* oam, IoRegisters* ioRegisters, Screen* screen);

    void Update(int cycles);
    void RenderFrame();
    
    // PPU timing constants
    static constexpr int CYCLES_PER_SCANLINE = 456;
    static constexpr int VISIBLE_SCANLINES = 144;
    static constexpr int VBLANK_SCANLINES = 10;
    static constexpr int TOTAL_SCANLINES = VISIBLE_SCANLINES + VBLANK_SCANLINES;

private:
    void RenderBackground();
    void RenderSprites();
    [[nodiscard]] byte GetTilePixel(word tileIndex, byte pixelX, byte pixelY) const;
    [[nodiscard]] word GetBackgroundPalette() const;
    void SetPixel(int x, int y, byte colorIndex);

    VRam* _vRam;
    Oam* _oam;
    IoRegisters* _ioRegisters;
    Screen* _screen;

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
};