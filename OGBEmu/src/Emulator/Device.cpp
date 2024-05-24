#include "Device.h"

#include <chrono>

#include "Core/Logger.h"
#include "Core/Utils.h"

namespace
{
    constexpr unsigned char DefaultSimulationFramesPerSecond = 64;
}

Device::Device(BootRom bootRom, Cartridge cartridge, const int framesPerSecond) : _bootRom(std::move(bootRom)),
    _cartridge(std::move(cartridge)),
    _bus(Bus(&_bootRom, &_cartridge, &_vRam, &_wRam, &_echoRam, &_oam, &_ioRegisters, &_hRam)),
    _cpu(&_bus),
    _framesPerSecond(framesPerSecond)
{
    if (!Utils::IsPowerOfTwo(_framesPerSecond))
    {
        LOG("Simulation frames per second must be a power of two, got " << _framesPerSecond << ", defaulting to " <<
            DefaultSimulationFramesPerSecond);
        _framesPerSecond = DefaultSimulationFramesPerSecond;
    }

    _frameTimeSeconds = 1. / _framesPerSecond;
    _maxCyclesPerFrame = Cpu::CpuClock * _frameTimeSeconds;
}

void Device::Run()
{
    constexpr double maxRunSeconds = 5000.;

    unsigned int totalCycles = 0;
    unsigned int totalFrames = 0;

    double runSeconds = 0;

    const auto runStartTime = std::chrono::steady_clock::now();
    while (runSeconds < maxRunSeconds)
    {
        const unsigned int cyclesDone = DoFrame();

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

unsigned Device::DoFrame()
{
    unsigned int cycleCount = 0;

    const auto frameStartTime = std::chrono::steady_clock::now();
    
    while (cycleCount < _maxCyclesPerFrame)
    {
        const byte cyclesExecuted = _cpu.Update();
        
        if (cyclesExecuted == 0)
        {
            return cycleCount;
        }

        cycleCount += cyclesExecuted;
    }
    
    const auto frameEndTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double> frameTime = frameEndTime - frameStartTime;

    WaitForNextFrame(frameTime.count());

    return cycleCount;
}

void Device::WaitForNextFrame(const double frameTimeSeconds) const
{
    const double remainingFrameTime = _frameTimeSeconds - frameTimeSeconds;

    if (remainingFrameTime < 0)
    {
        LOG("Running behind, last frame took: " << frameTimeSeconds << "s");
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
