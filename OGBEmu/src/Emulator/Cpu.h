#pragma once

class Bus;

class Cpu
{
public:
    explicit Cpu(Bus* bus, unsigned int framesPerSecond);

    void Run();

private:
    unsigned int DoFrame();
    void WaitForNextFrame(double frameTimeSeconds) const;

    bool DoCycle();
    
    static bool IsPowerOfTwo(const unsigned int value) { return value != 0 && (value & value - 1) == 0; }

    Bus* _bus;

    unsigned int _framesPerSecond;
    double _frameTimeSeconds;
    double _maxCyclesPerFrame;
};
