#include "Ppu.h"

#include "Core/Logger.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Screen.h"

Ppu::Ppu(VRam* vRam, Oam* oam, IoRegisters* ioRegisters, Screen* screen)
    : _vRam(vRam), _oam(oam), _ioRegisters(ioRegisters), _screen(screen)
{
    // Initialize frame buffer to white (Game Boy color index 0)
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        _frameBuffer[i] = 0;
    }
    
    LOG("PPU initialized");
}

void Ppu::Update(int cycles)
{
    // For now, simple frame-based rendering
    // TODO: Implement proper scanline timing (456 cycles per line)
    static int cycleCcount = 0;
    cycleCcount += cycles;
    
    // Render a frame every ~70224 cycles (Game Boy frame time)
    if (cycleCcount >= 70224)
    {
        RenderFrame();
        cycleCcount = 0;
    }
}

void Ppu::RenderFrame()
{
    // Check if LCD is enabled (LCDC bit 7)
    byte lcdc = _ioRegisters->Read(AddressConstants::LcdControl);
    if ((lcdc & 0x80) == 0)
    {
        // LCD is off - clear screen to white
        _screen->Clear();
        _screen->Present();
        return;
    }
    
    // Clear frame buffer
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        _frameBuffer[i] = 0; // White background
    }
    
    // Clear screen and render background using Screen's method
    _screen->Clear();
    _screen->RenderBackground(_vRam, _ioRegisters);
    _screen->Present();
}

void Ppu::RenderBackground()
{
    // TODO: Read LCD control register to check if background is enabled
    // For now, render a simple test pattern
    
    for (int tileY = 0; tileY < TILES_PER_COL; tileY++)
    {
        for (int tileX = 0; tileX < TILES_PER_ROW; tileX++)
        {
            // Read tile index from background map
            word mapAddress = BACKGROUND_MAP_START + (tileY * TILES_PER_ROW) + tileX;
            byte tileIndex = _vRam->Read(mapAddress);
            
            // Render 8x8 tile
            for (int pixelY = 0; pixelY < TILE_SIZE; pixelY++)
            {
                for (int pixelX = 0; pixelX < TILE_SIZE; pixelX++)
                {
                    byte colorIndex = GetTilePixel(tileIndex, pixelX, pixelY);
                    
                    int screenX = (tileX * TILE_SIZE) + pixelX;
                    int screenY = (tileY * TILE_SIZE) + pixelY;
                    
                    // Only render if within screen bounds
                    if (screenX < SCREEN_WIDTH && screenY < SCREEN_HEIGHT)
                    {
                        SetPixel(screenX, screenY, colorIndex);
                    }
                }
            }
        }
    }
}

void Ppu::RenderSprites()
{
    // TODO: Implement sprite rendering
    // Read OAM data and render sprites on top of background
}

byte Ppu::GetTilePixel(word tileIndex, byte pixelX, byte pixelY) const
{
    // Each tile is 16 bytes (8x8 pixels, 2 bits per pixel)
    word tileAddress = TILE_DATA_START + (tileIndex * 16);
    
    // Each row of pixels takes 2 bytes
    word rowAddress = tileAddress + (pixelY * 2);
    
    byte lowByte = _vRam->Read(rowAddress);
    byte highByte = _vRam->Read(rowAddress + 1);
    
    // Extract 2-bit color for this pixel
    byte bitPosition = 7 - pixelX;
    byte lowBit = (lowByte >> bitPosition) & 1;
    byte highBit = (highByte >> bitPosition) & 1;
    
    return (highBit << 1) | lowBit;
}

word Ppu::GetBackgroundPalette() const
{
    // Read BGP register (0xFF47) - Background Palette
    return _ioRegisters->Read(AddressConstants::BackgroundPalette);
}

void Ppu::SetPixel(int x, int y, byte colorIndex)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        _frameBuffer[y * SCREEN_WIDTH + x] = colorIndex;
    }
}