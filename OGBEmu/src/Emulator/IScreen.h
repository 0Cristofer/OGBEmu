#pragma once


// Forward declarations
class VRam;
class IoRegisters;
class Bus;
class Joypad;

// Interface for Screen to allow testing with mocks
class IScreen
{
public:
    virtual ~IScreen() = default;
    
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Clear() = 0;
    virtual void Present() = 0;
    virtual bool ShouldClose() const = 0;
    
    virtual void RenderBackground(const Bus& bus) = 0;
    virtual void DisplayFrameBuffer(const byte* frameBuffer, int width, int height) = 0;
    
    // Audio output (optional for implementations)
    virtual void PlayAudio(const float* audioBuffer, int bufferSize) {}
    
    // Input handling
    virtual void ProcessEvents(Joypad* joypad) {}
};