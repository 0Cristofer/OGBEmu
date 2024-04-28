#include "Cpu.h"

#include <chrono>

#include "Core/Logger.h"

#include "Bus.h"
#include "CpuConstants.h"

namespace
{
    constexpr unsigned char DefaultSimulationFramesPerSecond = 64;
}

Cpu::Cpu(Bus& bus, const unsigned int framesPerSecond) : _bus(bus), _framesPerSecond(framesPerSecond)
{
    if (!IsPowerOfTwo(_framesPerSecond))
    {
        LOG("Simulation frames per second must be a power of two, got " << _framesPerSecond << ", defaulting to " << DefaultSimulationFramesPerSecond);
        _framesPerSecond = DefaultSimulationFramesPerSecond;
    }

    _frameTimeSeconds = 1./_framesPerSecond;
    _maxCyclesPerFrame = CpuConstants::CpuClock * _frameTimeSeconds;

    _bus.EnableBootRom();
}

void Cpu::Run()
{
    constexpr double maxRunSeconds = 5.;
    
    unsigned int totalCycles = 0;
    unsigned int totalFrames = 0;
    
    double runSeconds = 0;
    
    const auto runStartTime = std::chrono::steady_clock::now();
    while(runSeconds < maxRunSeconds)
    {
        const unsigned int cyclesDone = DoFrame();

        const auto runCurrentTime = std::chrono::steady_clock::now();
        runSeconds = std::chrono::duration<double>(runCurrentTime - runStartTime).count();

        totalCycles += cyclesDone;
        totalFrames++;
    }

    LOG("Finished running. Ran for " << runSeconds << "s, with " << totalFrames << " frames and cycled " << totalCycles << " times");
}

unsigned int Cpu::DoFrame()
{
    unsigned int cycleCount = 0;
    
    const auto frameStartTime = std::chrono::steady_clock::now();
    while (cycleCount < _maxCyclesPerFrame)
    {
        DoCycle();
        cycleCount++;
    }
    const auto frameEndTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double> frameTime = frameEndTime - frameStartTime;

    WaitForNextFrame(frameTime.count());

    return cycleCount;
}

void Cpu::WaitForNextFrame(const double frameTimeSeconds) const
{
    const double remainingFrameTime = _frameTimeSeconds - frameTimeSeconds;

    if (remainingFrameTime < 0)
    {
        LOG("Running behind, last frame took: " << frameTimeSeconds << "s");
    }

    double remainingWaitTime = remainingFrameTime;
    
    const auto waitingStartTime = std::chrono::steady_clock::now();
    while(remainingWaitTime > 0)
    {
        const auto waitingEndTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double> waitingTime = waitingEndTime - waitingStartTime;
        remainingWaitTime = remainingFrameTime - waitingTime.count();
    }
}

void Cpu::DoCycle()
{
    byte value = _bus.Read(0x13);
}
