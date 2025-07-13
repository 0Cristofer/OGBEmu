#pragma once

#include <SDL3/SDL.h>
#include "Core/Definitions.h"
#include "IScreen.h"

class Bus;

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

    void RenderBackground(const Bus& bus) override;
    void DisplayFrameBuffer(const byte* frameBuffer, int width, int height) override;
    
    // Input handling
    void ProcessEvents(Joypad* joypad) override;
    
    // Audio functions
    bool InitializeAudio();
    void ShutdownAudio();
    void PlayAudio(const float* audioBuffer, int bufferSize) override;

private:
    SDL_Window* _window;
    SDL_Renderer* _renderer;
    bool _shouldClose;
    
    // Audio members
    SDL_AudioDeviceID _audioDevice;
    SDL_AudioStream* _audioStream;
    bool _audioInitialized;

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
    
    void RenderTile(int tileIndex, int x, int y, const Bus& bus, byte palette);
};
