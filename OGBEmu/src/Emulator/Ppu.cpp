#include "Ppu.h"

#include "Core/Logger.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/IScreen.h"
#include "Memory/Bus.h"

Ppu::Ppu(Bus* bus, IScreen* screen)
    : _bus(bus), _screen(screen), _currentCycles(0), _currentScanline(0), _currentMode(PpuMode::OamSearch)
{
    // Initialize frame buffer to white (Game Boy color index 0)
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        _frameBuffer[i] = 0;
    }
    
    // Synchronize internal scanline with LY register
    _currentScanline = _bus->Read(AddressConstants::LcdY);
    
    LOG("PPU initialized");
}

void Ppu::Update(int cycles)
{
    // Check if LCD is enabled
    byte lcdc = _bus->Read(AddressConstants::LcdControl);
    if ((lcdc & 0x80) == 0)
    {
        // LCD is disabled - reset PPU state
        _currentCycles = 0;
        _currentScanline = 0;
        _bus->Write(AddressConstants::LcdY, 0);
        return;
    }
    
    // Synchronize internal scanline with LY register at start of update
    _currentScanline = _bus->Read(AddressConstants::LcdY);
    
    _currentCycles += cycles;
    
    // Update PPU mode based on timing within scanline
    UpdatePpuMode();
    
    // Check if we've completed a scanline (456 cycles)
    if (_currentCycles >= CYCLES_PER_SCANLINE)
    {
        _currentCycles -= CYCLES_PER_SCANLINE;
        _currentScanline++;
        
        // Handle scanline overflow (154 total scanlines: 0-153)
        if (_currentScanline >= TOTAL_SCANLINES)
        {
            _currentScanline = 0;
        }
        
        // Update LY register
        _bus->Write(AddressConstants::LcdY, _currentScanline);
        
        // Update PPU mode for new scanline
        UpdatePpuMode();
        
        // Update STAT register and check for interrupts
        UpdateStatRegister();
        CheckStatInterrupts();
        
        // Render frame when entering VBlank (scanline 144)
        if (_currentScanline == VISIBLE_SCANLINES)
        {
            RenderFrame();
            
            // Trigger VBlank interrupt
            byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
            interruptFlag |= 0x01; // Set VBlank interrupt flag (bit 0)
            _bus->Write(AddressConstants::InterruptFlag, interruptFlag);
        }
    }
    else
    {
        // Update STAT register and check for interrupts mid-scanline
        UpdateStatRegister();
        CheckStatInterrupts();
    }
}

void Ppu::RenderFrame()
{
    // Check if LCD is enabled (LCDC bit 7)
    byte lcdc = _bus->Read(AddressConstants::LcdControl);
    if ((lcdc & 0x80) == 0)
    {
        // LCD is off - clear screen to white
        _screen->Clear();
        return;
    }
    
    // Clear frame buffer
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        _frameBuffer[i] = 0; // White background
    }
    
    // Render background if enabled
    if (lcdc & 0x01) // Bit 0: Background enable
    {
        RenderBackground();
    }
    
    // Render window layer if enabled
    if (lcdc & 0x20) // Bit 5: Window enable
    {
        RenderWindow();
    }
    
    // Render sprites if enabled
    if (lcdc & 0x02) // Bit 1: Sprite enable
    {
        RenderSprites();
    }
    
    // Display the completed frame buffer on screen
    _screen->Clear();
    _screen->DisplayFrameBuffer(_frameBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Ppu::RenderBackground()
{
    // Check if background is enabled (LCDC bit 0)
    byte lcdc = _bus->Read(AddressConstants::LcdControl);
    if ((lcdc & 0x01) == 0)
    {
        // Background disabled - fill with white (color index 0)
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        {
            _frameBuffer[i] = 0;
        }
        return;
    }
    
    for (int tileY = 0; tileY < TILES_PER_COL; tileY++)
    {
        for (int tileX = 0; tileX < TILES_PER_ROW; tileX++)
        {
            // Read tile index from background map
            word mapAddress = BACKGROUND_MAP_START + (tileY * TILES_PER_ROW) + tileX;
            byte tileIndex = _bus->Read(mapAddress);
            
            // Render 8x8 tile
            for (int pixelY = 0; pixelY < TILE_SIZE; pixelY++)
            {
                for (int pixelX = 0; pixelX < TILE_SIZE; pixelX++)
                {
                    byte colorIndex = GetTilePixel(tileIndex, pixelX, pixelY);
                    
                    // Apply background palette
                    byte bgp = _bus->Read(AddressConstants::BackgroundPalette);
                    byte finalColor = (bgp >> (colorIndex * 2)) & 0x03;
                    
                    int screenX = (tileX * TILE_SIZE) + pixelX;
                    int screenY = (tileY * TILE_SIZE) + pixelY;
                    
                    // Only render if within screen bounds
                    if (screenX < SCREEN_WIDTH && screenY < SCREEN_HEIGHT)
                    {
                        SetPixel(screenX, screenY, finalColor);
                    }
                }
            }
        }
    }
}

void Ppu::RenderSprites()
{
    byte lcdc = _bus->Read(AddressConstants::LcdControl);
    bool use8x16Sprites = (lcdc & 0x04) != 0; // Bit 2: Sprite size
    
    // Track which pixels have been drawn by sprites (for sprite-to-sprite priority)
    bool spritePixelDrawn[SCREEN_WIDTH * SCREEN_HEIGHT] = {false};
    
    // Game Boy can display up to 40 sprites
    // Render in reverse order so lower OAM index sprites appear on top
    for (int sprite = 39; sprite >= 0; sprite--)
    {
        // Each sprite entry is 4 bytes in OAM memory (0xFE00-0xFE9F)
        word oamAddress = AddressConstants::StartOamAddress + (sprite * 4);
        
        // Read sprite attributes
        int yPos = static_cast<int>(_bus->Read(oamAddress)) - 16; // Y position (adjusted)
        int xPos = static_cast<int>(_bus->Read(oamAddress + 1)) - 8; // X position (adjusted)
        byte tileIndex = _bus->Read(oamAddress + 2);
        byte attributes = _bus->Read(oamAddress + 3);
        
        // Determine sprite height
        int spriteHeight = use8x16Sprites ? 16 : 8;
        
        // Skip if sprite is completely off screen
        // Note: yPos and xPos can be negative due to the offset adjustments
        if (yPos >= SCREEN_HEIGHT || xPos >= SCREEN_WIDTH || 
            yPos <= -spriteHeight || xPos <= -8)
            continue;
            
        // Parse attributes
        bool flipY = (attributes & 0x40) != 0;
        bool flipX = (attributes & 0x20) != 0;
        bool behindBg = (attributes & 0x80) != 0; // 0 = above BG, 1 = behind BG
        byte palette = (attributes & 0x10) ? 1 : 0; // OBP0 or OBP1
        
        // For 8x16 sprites, ignore bit 0 of tile index
        if (use8x16Sprites)
        {
            tileIndex &= 0xFE;
        }
        
        // Render sprite pixels
        for (int py = 0; py < spriteHeight; py++)
        {
            for (int px = 0; px < 8; px++)
            {
                // Calculate screen position
                int screenX = xPos + px;
                int screenY = yPos + py;
                
                // Skip if pixel is off screen
                if (screenX < 0 || screenX >= SCREEN_WIDTH || 
                    screenY < 0 || screenY >= SCREEN_HEIGHT)
                    continue;
                
                // Apply flipping
                int tileX = flipX ? (7 - px) : px;
                int tileY = flipY ? (spriteHeight - 1 - py) : py;
                
                // For 8x16 sprites, calculate which tile to use
                byte currentTileIndex = tileIndex;
                if (use8x16Sprites && tileY >= 8)
                {
                    currentTileIndex |= 0x01;  // Use bottom tile (NN | $01)
                    tileY -= 8;                // Adjust Y within the 8x8 tile
                }
                
                // Get pixel color from tile
                byte colorIndex = GetSpritePixel(currentTileIndex, tileX, tileY);
                
                // Color 0 is transparent for sprites
                if (colorIndex == 0)
                    continue;
                
                // Check priority against background
                if (behindBg)
                {
                    // Sprite is behind background colors 1-3
                    int bufferIndex = screenY * SCREEN_WIDTH + screenX;
                    if (_frameBuffer[bufferIndex] != 0)
                        continue;
                }
                
                // Check if this pixel has already been drawn by a higher priority sprite
                int pixelIndex = screenY * SCREEN_WIDTH + screenX;
                if (spritePixelDrawn[pixelIndex])
                    continue;
                
                // Apply palette and set pixel
                word paletteAddress = palette ? AddressConstants::ObjectPalette1 : AddressConstants::ObjectPalette0;
                byte paletteData = _bus->Read(paletteAddress);
                byte finalColor = (paletteData >> (colorIndex * 2)) & 0x03;
                
                SetPixel(screenX, screenY, finalColor);
                spritePixelDrawn[pixelIndex] = true;
            }
        }
    }
}

byte Ppu::GetTilePixel(word tileIndex, byte pixelX, byte pixelY) const
{
    // Each tile is 16 bytes (8x8 pixels, 2 bits per pixel)
    word tileAddress = TILE_DATA_START + (tileIndex * 16);
    
    // Each row of pixels takes 2 bytes
    word rowAddress = tileAddress + (pixelY * 2);
    
    byte lowByte = _bus->Read(rowAddress);
    byte highByte = _bus->Read(rowAddress + 1);
    
    // Extract 2-bit color for this pixel
    byte bitPosition = 7 - pixelX;
    byte lowBit = (lowByte >> bitPosition) & 1;
    byte highBit = (highByte >> bitPosition) & 1;
    
    return (highBit << 1) | lowBit;
}

byte Ppu::GetSpritePixel(word tileIndex, byte pixelX, byte pixelY) const
{
    // Sprites always use tile data from 0x8000
    word tileAddress = 0x8000 + (tileIndex * 16);
    
    // Each row of pixels takes 2 bytes
    word rowAddress = tileAddress + (pixelY * 2);
    
    byte lowByte = _bus->Read(rowAddress);
    byte highByte = _bus->Read(rowAddress + 1);
    
    // Extract 2-bit color for this pixel
    byte bitPosition = 7 - pixelX;
    byte lowBit = (lowByte >> bitPosition) & 1;
    byte highBit = (highByte >> bitPosition) & 1;
    
    return (highBit << 1) | lowBit;
}

word Ppu::GetBackgroundPalette() const
{
    // Read BGP register (0xFF47) - Background Palette
    return _bus->Read(AddressConstants::BackgroundPalette);
}

void Ppu::SetPixel(int x, int y, byte colorIndex)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        _frameBuffer[y * SCREEN_WIDTH + x] = colorIndex;
    }
}

void Ppu::RenderWindow()
{
    byte lcdc = _bus->Read(AddressConstants::LcdControl);
    if ((lcdc & 0x20) == 0) // Bit 5: Window enable
        return;
        
    byte windowY = _bus->Read(AddressConstants::WindowY);
    byte windowX = _bus->Read(AddressConstants::WindowX) - 7; // WX is offset by 7
    
    // Window is not visible if WY > 143 or WX > 166
    if (windowY > 143 || windowX > 159)
        return;
        
    // Determine window tile map
    word windowMapStart = (lcdc & 0x40) ? 0x9C00 : 0x9800; // Bit 6: Window tile map
    
    // Determine tile data mode
    bool signedMode = (lcdc & 0x10) == 0; // Bit 4: BG/Window tile data
    word tileDataStart = signedMode ? 0x9000 : 0x8000;
    
    // Render window tiles
    for (int screenY = windowY; screenY < SCREEN_HEIGHT; screenY++)
    {
        int windowTileY = (screenY - windowY) / 8;
        int tilePixelY = (screenY - windowY) % 8;
        
        for (int screenX = windowX; screenX < SCREEN_WIDTH; screenX++)
        {
            int windowTileX = (screenX - windowX) / 8;
            int tilePixelX = (screenX - windowX) % 8;
            
            // Read tile index from window map
            word mapAddress = windowMapStart + (windowTileY * 32) + windowTileX;
            byte tileIndex = _bus->Read(mapAddress);
            
            // Handle signed tile indices
            word tileOffset;
            if (signedMode)
            {
                signed_byte signedIndex = static_cast<signed_byte>(tileIndex);
                tileOffset = signedIndex * 16;
            }
            else
            {
                tileOffset = tileIndex * 16;
            }
            
            // Read tile data
            word tileAddress = tileDataStart + tileOffset;
            word rowAddress = tileAddress + (tilePixelY * 2);
            
            byte lowByte = _bus->Read(rowAddress);
            byte highByte = _bus->Read(rowAddress + 1);
            
            // Extract pixel color
            byte bitPosition = 7 - tilePixelX;
            byte lowBit = (lowByte >> bitPosition) & 1;
            byte highBit = (highByte >> bitPosition) & 1;
            byte colorIndex = (highBit << 1) | lowBit;
            
            // Apply background palette
            byte bgp = _bus->Read(AddressConstants::BackgroundPalette);
            byte finalColor = (bgp >> (colorIndex * 2)) & 0x03;
            
            SetPixel(screenX, screenY, finalColor);
        }
    }
}

void Ppu::UpdatePpuMode()
{
    PpuMode previousMode = _currentMode;
    
    if (_currentScanline >= VISIBLE_SCANLINES)
    {
        // VBlank mode (scanlines 144-153)
        _currentMode = PpuMode::VBlank;
    }
    else
    {
        // Visible scanlines (0-143) - determine mode based on cycle timing
        if (_currentCycles < OAM_SEARCH_CYCLES)
        {
            _currentMode = PpuMode::OamSearch;  // Mode 2: First 80 cycles
        }
        else if (_currentCycles < OAM_SEARCH_CYCLES + DRAWING_CYCLES)
        {
            _currentMode = PpuMode::Drawing;    // Mode 3: Cycles 80-251
        }
        else
        {
            _currentMode = PpuMode::HBlank;     // Mode 0: Cycles 252-455
        }
    }
}

void Ppu::UpdateStatRegister()
{
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    
    // Clear mode bits (bits 0-1) and LY=LYC flag (bit 2)
    statRegister &= 0xF8;
    
    // Set current mode bits (bits 0-1)
    statRegister |= static_cast<byte>(_currentMode);
    
    // Set LY=LYC comparison flag (bit 2)
    if (IsLyLycMatch())
    {
        statRegister |= 0x04;
    }
    
    _bus->Write(AddressConstants::LcdStatus, statRegister);
}

void Ppu::CheckStatInterrupts()
{
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    bool shouldTriggerStatInterrupt = false;
    
    // Check HBlank interrupt (bit 3)
    if ((statRegister & 0x08) && _currentMode == PpuMode::HBlank)
    {
        shouldTriggerStatInterrupt = true;
    }
    
    // Check VBlank interrupt (bit 4) - Note: VBlank also triggers dedicated VBlank interrupt
    if ((statRegister & 0x10) && _currentMode == PpuMode::VBlank)
    {
        shouldTriggerStatInterrupt = true;
    }
    
    // Check OAM interrupt (bit 5)
    if ((statRegister & 0x20) && _currentMode == PpuMode::OamSearch)
    {
        shouldTriggerStatInterrupt = true;
    }
    
    // Check LY=LYC interrupt (bit 6)
    if ((statRegister & 0x40) && IsLyLycMatch())
    {
        shouldTriggerStatInterrupt = true;
    }
    
    if (shouldTriggerStatInterrupt)
    {
        // Trigger STAT interrupt
        byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
        interruptFlag |= 0x02; // Set LCD/STAT interrupt flag (bit 1)
        _bus->Write(AddressConstants::InterruptFlag, interruptFlag);
    }
}

bool Ppu::IsLyLycMatch() const
{
    byte ly = _bus->Read(AddressConstants::LcdY);
    byte lyc = _bus->Read(AddressConstants::LcdYCompare);
    return ly == lyc;
}