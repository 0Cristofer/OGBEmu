#include "Screen.h"

#include "Core/Logger.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Memory/Bus.h"
#include "Emulator/Joypad.h"

Screen::Screen() : _window(nullptr), _renderer(nullptr), _shouldClose(false), _audioDevice(0), _audioStream(nullptr), _audioInitialized(false)
{
}

Screen::~Screen()
{
    Shutdown();
}

bool Screen::Initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        ERROR("SDL could not initialize! SDL_Error: " << SDL_GetError());
        return false;
    }

    _window = SDL_CreateWindow("OGBEmu",
        SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
        0);

    if (_window == nullptr)
    {
        ERROR("Window could not be created! SDL_Error: " << SDL_GetError());
        SDL_Quit();
        return false;
    }

    _renderer = SDL_CreateRenderer(_window, nullptr);
    if (_renderer == nullptr)
    {
        ERROR("Renderer could not be created! SDL_Error: " << SDL_GetError());
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }

    // Initialize audio
    if (!InitializeAudio())
    {
        ERROR("Failed to initialize audio");
        // Continue without audio
    }
    
    LOG("Screen initialized successfully");
    return true;
}

void Screen::Shutdown()
{
    ShutdownAudio();
    
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
}

void Screen::ProcessEvents(Joypad* joypad)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            _shouldClose = true;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
        {
            bool pressed = (event.type == SDL_EVENT_KEY_DOWN);
            
            // Map keyboard keys to Game Boy buttons
            switch (event.key.key)
            {
                case SDLK_RIGHT:  // Right arrow -> Right
                    if (joypad) joypad->SetButtonState(Joypad::Right, pressed);
                    break;
                case SDLK_LEFT:   // Left arrow -> Left
                    if (joypad) joypad->SetButtonState(Joypad::Left, pressed);
                    break;
                case SDLK_UP:     // Up arrow -> Up
                    if (joypad) joypad->SetButtonState(Joypad::Up, pressed);
                    break;
                case SDLK_DOWN:   // Down arrow -> Down
                    if (joypad) joypad->SetButtonState(Joypad::Down, pressed);
                    break;
                case SDLK_Z:      // Z -> A button
                    if (joypad) joypad->SetButtonState(Joypad::A, pressed);
                    break;
                case SDLK_X:      // X -> B button
                    if (joypad) joypad->SetButtonState(Joypad::B, pressed);
                    break;
                case SDLK_SPACE:  // Space -> Select
                    if (joypad) joypad->SetButtonState(Joypad::Select, pressed);
                    break;
                case SDLK_RETURN: // Enter -> Start
                    if (joypad) joypad->SetButtonState(Joypad::Start, pressed);
                    break;
                default:
                    break;
            }
        }
    }
}

bool Screen::ShouldClose() const
{
    return _shouldClose;
}

void Screen::RenderBackground(const Bus& bus)
{
    // Check LCDC register to see if background is enabled (bit 0)
    byte lcdc = bus.Read(AddressConstants::LcdControl);
    if ((lcdc & 0x01) == 0)
    {
        // Background disabled - clear to white
        return;
    }
    
    // Get background palette (BGP register)
    byte bgp = bus.Read(AddressConstants::BackgroundPalette);
    
    // Get scroll values
    byte scrollY = bus.Read(AddressConstants::ScrollY);
    byte scrollX = bus.Read(AddressConstants::ScrollX);
    
    // Background tile map starts at 0x9800 (can also be 0x9C00 based on LCDC bit 3)
    int bgMapStart = (lcdc & 0x08) ? 0x9C00 : 0x9800;
    
    // Render 20x18 tiles (visible area is 160x144 pixels)
    for (int tileY = 0; tileY < 18; ++tileY)
    {
        for (int tileX = 0; tileX < 20; ++tileX)
        {
            // Apply scrolling
            int mapX = (tileX + (scrollX / 8)) % 32;
            int mapY = (tileY + (scrollY / 8)) % 32;
            
            // Get tile index from background map
            const int mapIndex = (mapY * BACKGROUND_WIDTH) + mapX;
            const int tileIndex = bus.Read(bgMapStart + mapIndex);
            
            // Render the tile with palette
            RenderTile(tileIndex, tileX * TILE_SIZE, tileY * TILE_SIZE, bus, bgp);
        }
    }
}

void Screen::RenderTile(int tileIndex, int x, int y, const Bus& bus, byte palette)
{
    // Tile data starts at 0x8000
    constexpr int TILE_DATA_START = 0x8000;
    const int tileDataAddress = TILE_DATA_START + (tileIndex * 16); // 16 bytes per tile
    
    // Each tile is 8x8 pixels, 2 bits per pixel
    for (int row = 0; row < TILE_SIZE; ++row)
    {
        // Each row is 2 bytes (low and high bits)
        const byte lowByte = bus.Read(tileDataAddress + (row * 2));
        const byte highByte = bus.Read(tileDataAddress + (row * 2) + 1);
        
        for (int col = 0; col < TILE_SIZE; ++col)
        {
            // Get 2-bit color value for this pixel
            const int bit = 7 - col;
            const int rawColorIndex = ((highByte >> bit) & 1) << 1 | ((lowByte >> bit) & 1);
            
            // Apply palette mapping: palette has 2 bits per color (bits 1-0 for color 0, 3-2 for color 1, etc.)
            const int paletteColorIndex = (palette >> (rawColorIndex * 2)) & 0x03;
            
            // Set pixel color using palette-mapped color
            const SDL_Color& color = PALETTE[paletteColorIndex];
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

void Screen::DisplayFrameBuffer(const byte* frameBuffer, int width, int height)
{
    if (!frameBuffer || !_renderer)
        return;
        
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Get color index from frame buffer
            byte colorIndex = frameBuffer[y * width + x];
            
            // Map to palette color
            const SDL_Color& color = PALETTE[colorIndex & 0x03];
            SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
            
            // Draw pixel (scaled up by WINDOW_SCALE)
            SDL_FRect rect = {
                static_cast<float>(x * WINDOW_SCALE),
                static_cast<float>(y * WINDOW_SCALE),
                static_cast<float>(WINDOW_SCALE),
                static_cast<float>(WINDOW_SCALE)
            };
            SDL_RenderFillRect(_renderer, &rect);
        }
    }
}bool Screen::InitializeAudio()
{
    // Set up audio specification
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = 48000;        // 48kHz sample rate
    desired.format = SDL_AUDIO_F32; // 32-bit float samples
    desired.channels = 2;        // Stereo
    
    // Open audio device
    _audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    
    if (_audioDevice == 0)
    {
        ERROR("Failed to open audio device: " << SDL_GetError());
        return false;
    }
    
    // Create audio stream
    _audioStream = SDL_CreateAudioStream(&desired, &desired);
    if (!_audioStream)
    {
        ERROR("Failed to create audio stream: " << SDL_GetError());
        SDL_CloseAudioDevice(_audioDevice);
        _audioDevice = 0;
        return false;
    }
    
    // Bind stream to device
    if (!SDL_BindAudioStream(_audioDevice, _audioStream))
    {
        ERROR("Failed to bind audio stream: " << SDL_GetError());
        SDL_DestroyAudioStream(_audioStream);
        SDL_CloseAudioDevice(_audioDevice);
        _audioDevice = 0;
        return false;
    }
    
    _audioInitialized = true;
    LOG("Audio initialized successfully: " << desired.freq << "Hz, " << (int)desired.channels << " channels");
    return true;
}

void Screen::ShutdownAudio()
{
    if (_audioInitialized)
    {
        SDL_CloseAudioDevice(_audioDevice);
        _audioDevice = 0;
        _audioInitialized = false;
        LOG("Audio shutdown completed");
    }
}

void Screen::PlayAudio(const float* audioBuffer, int bufferSize)
{
    if (!_audioInitialized || !_audioStream || !audioBuffer || bufferSize <= 0)
        return;
        
    // Put audio data into stream
    if (!SDL_PutAudioStreamData(_audioStream, audioBuffer, bufferSize * sizeof(float)))
    {
        // Optionally log errors, but don't spam the log
        static int errorCount = 0;
        if (errorCount < 5)
        {
            ERROR("Failed to queue audio: " << SDL_GetError());
            errorCount++;
        }
    }
}