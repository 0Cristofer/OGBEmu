#include "Device.h"

#include <chrono>

#include "Core/Logger.h"
#include "Core/Utils.h"

namespace
{
    constexpr unsigned char DefaultSimulationFramesPerSecond = 64;
}

Device::Device(const std::vector<byte>& bootRomBytes, const std::vector<byte>& cartridgeBytes, const int framesPerSecond, const double timeoutSeconds, IScreen* screen) : _bootRom(bootRomBytes),
                                                                                                                            _cartridge(cartridgeBytes),
                                                                                                                            _bus(Bus(&_bootRom, &_cartridge, &_vRam, &_wRam, &_wRamCgb, &_echoRam, &_oam, &_ioRegisters, &_hRam)),
                                                                                                                            _cpu(&_bus),
                                                                                                                            _screen(screen),
                                                                                                                            _ppu(&_bus, _screen),
                                                                                                                            _timer(&_ioRegisters),
                                                                                                                            _joypad(&_ioRegisters),
                                                                                                                            _dma(&_bus),
                                                                                                                            _apu(&_ioRegisters),
                                                                                                                            _framesPerSecond(framesPerSecond),
                                                                                                                            _timeoutSeconds(timeoutSeconds)
{
    if (!Utils::IsPowerOfTwo(_framesPerSecond))
    {
        ERROR("Simulation frames per second must be a power of two, got " << _framesPerSecond << ", defaulting to " <<
            DefaultSimulationFramesPerSecond);
        _framesPerSecond = DefaultSimulationFramesPerSecond;
    }

    _frameTimeSeconds = 1. / _framesPerSecond;
    _maxCyclesPerFrame = Cpu::CpuClock * _frameTimeSeconds;
    
    if (!_screen->Initialize())
    {
        ERROR("Failed to initialize screen");
    }
    
    // Set DMA pointer in Bus
    _bus.SetDma(&_dma);
    
    // Connect APU to IoRegisters for audio register callbacks
    _ioRegisters.SetApu(&_apu);
    
    // Enable APU by default
    _apu.SetEnabled(true);
    
    // Set up CPU component references for cycle-accurate updates
    _cpu.SetComponents(&_timer, &_ppu, &_apu, &_dma, &_joypad, _screen);
}

bool Device::IsValid() const
{
    return _bootRom.IsValid() && _cartridge.IsValid();
}

void Device::Run()
{
    if (!IsValid())
        return;

    if (_timeoutSeconds > 0.0)
    {
        LOG("Running with timeout: " << _timeoutSeconds << "s");
    }
    else
    {
        LOG("Running indefinitely (no timeout)");
    }

    unsigned long totalCycles = 0;
    unsigned long totalFrames = 0;

    double runSeconds = 0;

    const auto runStartTime = std::chrono::steady_clock::now();
    while ((_timeoutSeconds == 0.0 || runSeconds < _timeoutSeconds) && !_screen->ShouldClose())
    {
        const unsigned long cyclesDone = DoFrame();

        if (cyclesDone == 0)
        {
            return;
        }

        const auto runCurrentTime = std::chrono::steady_clock::now();
        runSeconds = std::chrono::duration<double>(runCurrentTime - runStartTime).count();

        totalCycles += cyclesDone;
        totalFrames++;
    }

    LOG("Finished running. Ran for " << runSeconds << "s, with " << totalFrames << " frames and cycled " << totalCycles
        << " times");
}

unsigned long Device::DoFrame()
{
    unsigned long cycleCount = 0;

    const auto frameStartTime = std::chrono::steady_clock::now();
    
    // Process input events once per frame
    _screen->ProcessEvents(&_joypad);
    
    while (cycleCount < _maxCyclesPerFrame)
    {
        const byte cyclesExecuted = _cpu.Update();
        
        if (cyclesExecuted == 0)
        {
            return cycleCount;
        }

        cycleCount += cyclesExecuted;
    }
     
    // PPU handles screen rendering now
    const auto frameEndTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double> frameTime = frameEndTime - frameStartTime;

    WaitForNextFrame(frameTime.count());

    _screen->Present();

    return cycleCount;
}

void Device::WaitForNextFrame(const double frameTimeSeconds) const
{
    const double remainingFrameTime = _frameTimeSeconds - frameTimeSeconds;

    if (remainingFrameTime < 0)
    {
        //LOG("Running behind, last frame took: " << frameTimeSeconds << "s, but should take " << _frameTimeSeconds << "s");
    }

    double remainingWaitTime = remainingFrameTime;

    const auto waitingStartTime = std::chrono::steady_clock::now();
    while (remainingWaitTime > 0)
    {
        const auto waitingEndTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double> waitingTime = waitingEndTime - waitingStartTime;
        remainingWaitTime = remainingFrameTime - waitingTime.count();
    }
}
