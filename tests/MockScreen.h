#pragma once

#include "Core/Definitions.h"
#include "Emulator/IScreen.h"

// Mock Screen class for testing without SDL dependency
class MockScreen : public IScreen
{
public:
    MockScreen() : _shouldClose(false) {}
    ~MockScreen() override = default;

    bool Initialize() override { return true; }
    void Shutdown() override {}
    void Clear() override {}
    void Present() override {}
    bool ShouldClose() const override { return _shouldClose; }
    
    void RenderBackground(const VRam* vram, const IoRegisters* ioRegisters) override {}

private:
    bool _shouldClose;
    static constexpr int SCREEN_WIDTH = 160;
    static constexpr int SCREEN_HEIGHT = 144;
};