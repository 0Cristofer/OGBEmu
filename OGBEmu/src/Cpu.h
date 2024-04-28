#pragma once

#include "Bus.h"

class Cpu
{
public:
    explicit Cpu(Bus& bus, const unsigned int framesPerSecond);

    void Run();

private:
    unsigned int DoFrame();
    void WaitForNextFrame(const double frameTimeSeconds) const;

    void DoCycle();
    
    static bool IsPowerOfTwo(const unsigned int value) { return value != 0 && (value & value - 1) == 0; }

    Bus& _bus;

    unsigned int _framesPerSecond;
    double _frameTimeSeconds;
    double _maxCyclesPerFrame;
};
