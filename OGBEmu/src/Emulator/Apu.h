#pragma once

#include "Core/Definitions.h"
#include <vector>

class IoRegisters;

class Apu
{
public:
    Apu(IoRegisters* ioRegisters);
    ~Apu();
    
    // Update APU with given CPU cycles
    void Update(int cycles);
    
    // Reset APU state
    void Reset();
    
    // Enable/disable audio output
    void SetEnabled(bool enabled) { _enabled = enabled; }
    bool IsEnabled() const { return _enabled; }
    
    // Register write callback
    void WriteRegister(word address, byte value);
    
    // Get audio buffer for output
    const std::vector<float>& GetAudioBuffer() const { return _audioBuffer; }
    void ClearAudioBuffer() { _audioBuffer.clear(); }
    
private:
    IoRegisters* _ioRegisters;
    bool _enabled;
    
    // Frame sequencer for length, volume, and sweep updates
    int _frameSequencerCounter;
    int _frameSequencerStep;
    static constexpr int FRAME_SEQUENCER_RATE = 8192; // 512 Hz (4194304 / 512)
    
    // Audio sample generation
    int _sampleCounter;
    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int CPU_FREQUENCY = 4194304;
    static constexpr int CYCLES_PER_SAMPLE = CPU_FREQUENCY / SAMPLE_RATE;
    
    // APU frequency constants
    static constexpr int SOUND_FREQUENCY_BASE = 131072; // Base frequency for calculation
    
    // Channel 1: Square wave with sweep
    struct SquareChannel
    {
        bool enabled;
        bool dacEnabled;
        bool lengthEnabled;
        int lengthCounter;
        int frequency;
        int volume;
        int volumeInitial;
        bool volumeIncrease;
        int volumePeriod;
        int volumeTimer;
        int duty;
        int dutyPosition;
        int timer;
        
        // Sweep (Channel 1 only)
        int sweepPeriod;
        bool sweepIncrease;
        int sweepShift;
        int sweepTimer;
        bool sweepEnabled;
    };
    
    // Channel 2: Square wave (no sweep)
    struct SquareChannelSimple
    {
        bool enabled;
        bool dacEnabled;
        bool lengthEnabled;
        int lengthCounter;
        int frequency;
        int volume;
        int volumeInitial;
        bool volumeIncrease;
        int volumePeriod;
        int volumeTimer;
        int duty;
        int dutyPosition;
        int timer;
    };
    
    // Channel 3: Wave channel
    struct WaveChannel
    {
        bool enabled;
        bool dacEnabled;
        bool lengthEnabled;
        int lengthCounter;
        int frequency;
        int outputLevel;
        int position;
        int timer;
        byte wavePattern[32]; // 32 4-bit samples
    };
    
    // Channel 4: Noise channel
    struct NoiseChannel
    {
        bool enabled;
        bool dacEnabled;
        bool lengthEnabled;
        int lengthCounter;
        int volume;
        int volumeInitial;
        bool volumeIncrease;
        int volumePeriod;
        int volumeTimer;
        int clockShift;
        bool widthMode;
        int divisorCode;
        int timer;
        word lfsr; // Linear Feedback Shift Register
    };
    
    SquareChannel _channel1;
    SquareChannelSimple _channel2;
    WaveChannel _channel3;
    NoiseChannel _channel4;
    
    // Update frame sequencer
    void UpdateFrameSequencer();
    
    // Update individual channels
    void UpdateChannel1();
    void UpdateChannel2();
    void UpdateChannel3();
    void UpdateChannel4();
    
    // Generate audio sample
    void GenerateSample();
    
    // Channel sample generation
    int GetChannel1Sample();
    int GetChannel2Sample();
    int GetChannel3Sample();
    int GetChannel4Sample();
    
    // Register read/write handlers
    void WriteNR10(byte value);
    void WriteNR11(byte value);
    void WriteNR12(byte value);
    void WriteNR13(byte value);
    void WriteNR14(byte value);
    void WriteNR21(byte value);
    void WriteNR22(byte value);
    void WriteNR23(byte value);
    void WriteNR24(byte value);
    void WriteNR30(byte value);
    void WriteNR31(byte value);
    void WriteNR32(byte value);
    void WriteNR33(byte value);
    void WriteNR34(byte value);
    void WriteNR41(byte value);
    void WriteNR42(byte value);
    void WriteNR43(byte value);
    void WriteNR44(byte value);
    void WriteNR50(byte value);
    void WriteNR51(byte value);
    void WriteNR52(byte value);
    
    // Trigger channel restart
    void TriggerChannel1();
    void TriggerChannel2();
    void TriggerChannel3();
    void TriggerChannel4();
    
    // Sweep functions for Channel 1
    void UpdateSweep();
    int CalculateSweepFrequency();
    
    // Duty cycle patterns (4 patterns, 8 steps each)
    static constexpr byte DUTY_PATTERNS[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
        {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
        {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
        {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
    };
    
    // Master control
    bool _masterEnabled;
    byte _leftVolume;
    byte _rightVolume;
    byte _leftEnable;
    byte _rightEnable;
    
    // Initialize APU registers
    void InitializeRegisters();
    
    // Audio output interface
    std::vector<float> _audioBuffer;
    static constexpr int BUFFER_SIZE = 1024;
};