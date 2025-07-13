#include "Apu.h"

#include "Core/Logger.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"

Apu::Apu(IoRegisters* ioRegisters)
    : _ioRegisters(ioRegisters), _enabled(false), _frameSequencerCounter(0), 
      _frameSequencerStep(0), _sampleCounter(0)
{
    Reset();
    LOG("APU initialized");
}

Apu::~Apu()
{
}

void Apu::Update(int cycles)
{
    if (!_enabled)
    {
        return;
    }
    
    // Update frame sequencer (controls length counters, volume envelopes, and sweeps)
    _frameSequencerCounter += cycles;
    if (_frameSequencerCounter >= FRAME_SEQUENCER_RATE)
    {
        _frameSequencerCounter -= FRAME_SEQUENCER_RATE;
        UpdateFrameSequencer();
    }
    
    // Update channels with cycles
    for (int i = 0; i < cycles; i++)
    {
        UpdateChannel1();
        UpdateChannel2();
        UpdateChannel3();
        UpdateChannel4();
    }
    
    // Generate audio samples
    _sampleCounter += cycles;
    if (_sampleCounter >= CYCLES_PER_SAMPLE)
    {
        _sampleCounter -= CYCLES_PER_SAMPLE;
        GenerateSample();
    }
}

void Apu::Reset()
{
    _frameSequencerCounter = 0;
    _frameSequencerStep = 0;
    _sampleCounter = 0;
    
    // Reset channel states
    _channel1 = {false, false, false, 0, 0, 0, 0, false, 0, 0, 0, 0, 0, 0, false, 0, 0, false};
    _channel2 = {false, false, false, 0, 0, 0, 0, false, 0, 0, 0, 0, 0};
    _channel3 = {false, false, false, 0, 0, 0, 0, 0};
    _channel4 = {false, false, false, 0, 0, 0, false, 0, 0, 0, false, 0, 0, 0x7FFF};
    
    // Initialize wave pattern to zeros
    for (int i = 0; i < 32; i++)
    {
        _channel3.wavePattern[i] = 0;
    }
    
    _masterEnabled = true;
    _leftVolume = 7;
    _rightVolume = 7;
    _leftEnable = (0xF3 >> 4) & 0x0F;  // Left channel enables (bits 7-4)
    _rightEnable = 0xF3 & 0x0F;        // Right channel enables (bits 3-0)
    
    _audioBuffer.reserve(BUFFER_SIZE);
    
    InitializeRegisters();
}

// Register write implementations
void Apu::WriteNR52(byte value)
{
    _masterEnabled = (value & 0x80) != 0;
    if (!_masterEnabled)
    {
        // Disable all channels when master disabled
        _channel1.enabled = false;
        _channel2.enabled = false;
        _channel3.enabled = false;
        _channel4.enabled = false;
    }
}

void Apu::WriteNR14(byte value)
{
    _channel1.frequency = (_channel1.frequency & 0xFF) | ((value & 0x07) << 8);
    _channel1.lengthEnabled = (value & 0x40) != 0;
    
    if (value & 0x80) // Trigger bit
    {
        TriggerChannel1();
    }
}

void Apu::WriteNR24(byte value)
{
    _channel2.frequency = (_channel2.frequency & 0xFF) | ((value & 0x07) << 8);
    _channel2.lengthEnabled = (value & 0x40) != 0;
    
    if (value & 0x80) // Trigger bit
    {
        TriggerChannel2();
    }
}

void Apu::WriteNR51(byte value)
{
    _leftEnable = (value >> 4) & 0x0F;
    _rightEnable = value & 0x0F;
}

void Apu::WriteNR50(byte value)
{
    _leftVolume = (value >> 4) & 0x07;
    _rightVolume = value & 0x07;
}

// Placeholder implementations for other registers
void Apu::WriteNR10(byte value) { /* TODO: Implement sweep */ }
void Apu::WriteNR11(byte value) { /* TODO: Implement length/duty */ }
void Apu::WriteNR12(byte value) { /* TODO: Implement volume */ }
void Apu::WriteNR13(byte value) { /* TODO: Implement freq low */ }
void Apu::WriteNR21(byte value) { /* TODO: Implement length/duty */ }
void Apu::WriteNR22(byte value) { /* TODO: Implement volume */ }
void Apu::WriteNR23(byte value) { /* TODO: Implement freq low */ }
void Apu::WriteNR30(byte value) { /* TODO: Implement wave enable */ }
void Apu::WriteNR31(byte value) { /* TODO: Implement wave length */ }
void Apu::WriteNR32(byte value) { /* TODO: Implement wave level */ }
void Apu::WriteNR33(byte value) { /* TODO: Implement wave freq low */ }
void Apu::WriteNR34(byte value) { /* TODO: Implement wave freq high */ }
void Apu::WriteNR41(byte value) { /* TODO: Implement noise length */ }
void Apu::WriteNR42(byte value) { /* TODO: Implement noise volume */ }
void Apu::WriteNR43(byte value) { /* TODO: Implement noise freq */ }
void Apu::WriteNR44(byte value) { /* TODO: Implement noise control */ }

void Apu::WriteRegister(word address, byte value)
{
    switch (address)
    {
        // Channel 1 registers
        case 0xFF10: WriteNR10(value); break; // NR10 - Sweep
        case 0xFF11: WriteNR11(value); break; // NR11 - Length/Duty
        case 0xFF12: WriteNR12(value); break; // NR12 - Volume
        case 0xFF13: WriteNR13(value); break; // NR13 - Frequency Low
        case 0xFF14: WriteNR14(value); break; // NR14 - Frequency High/Control
        
        // Channel 2 registers
        case 0xFF16: WriteNR21(value); break; // NR21 - Length/Duty
        case 0xFF17: WriteNR22(value); break; // NR22 - Volume
        case 0xFF18: WriteNR23(value); break; // NR23 - Frequency Low
        case 0xFF19: WriteNR24(value); break; // NR24 - Frequency High/Control
        
        // Channel 3 registers
        case 0xFF1A: WriteNR30(value); break; // NR30 - Enable
        case 0xFF1B: WriteNR31(value); break; // NR31 - Length
        case 0xFF1C: WriteNR32(value); break; // NR32 - Output Level
        case 0xFF1D: WriteNR33(value); break; // NR33 - Frequency Low
        case 0xFF1E: WriteNR34(value); break; // NR34 - Frequency High/Control
        
        // Channel 4 registers
        case 0xFF20: WriteNR41(value); break; // NR41 - Length
        case 0xFF21: WriteNR42(value); break; // NR42 - Volume
        case 0xFF22: WriteNR43(value); break; // NR43 - Frequency/Noise
        case 0xFF23: WriteNR44(value); break; // NR44 - Control
        
        // Control registers
        case 0xFF24: WriteNR50(value); break; // NR50 - Master Volume
        case 0xFF25: WriteNR51(value); break; // NR51 - Sound Panning
        case 0xFF26: WriteNR52(value); break; // NR52 - Sound Enable
        
        // Wave pattern RAM (0xFF30-0xFF3F)
        default:
            if (address >= 0xFF30 && address <= 0xFF3F)
            {
                int index = (address - 0xFF30) * 2;
                _channel3.wavePattern[index] = (value >> 4) & 0x0F;
                _channel3.wavePattern[index + 1] = value & 0x0F;
            }
            break;
    }
}

void Apu::UpdateFrameSequencer()
{
    // Frame sequencer has 8 steps (0-7)
    _frameSequencerStep = (_frameSequencerStep + 1) & 7;
    
    // Step 0, 2, 4, 6: Length counter
    if ((_frameSequencerStep & 1) == 0)
    {
        // Update length counters for all channels
        if (_channel1.lengthEnabled && _channel1.lengthCounter > 0)
        {
            _channel1.lengthCounter--;
            if (_channel1.lengthCounter == 0)
            {
                _channel1.enabled = false;
            }
        }
        
        if (_channel2.lengthEnabled && _channel2.lengthCounter > 0)
        {
            _channel2.lengthCounter--;
            if (_channel2.lengthCounter == 0)
            {
                _channel2.enabled = false;
            }
        }
        
        if (_channel3.lengthEnabled && _channel3.lengthCounter > 0)
        {
            _channel3.lengthCounter--;
            if (_channel3.lengthCounter == 0)
            {
                _channel3.enabled = false;
            }
        }
        
        if (_channel4.lengthEnabled && _channel4.lengthCounter > 0)
        {
            _channel4.lengthCounter--;
            if (_channel4.lengthCounter == 0)
            {
                _channel4.enabled = false;
            }
        }
    }
    
    // Step 7: Volume envelope
    if (_frameSequencerStep == 7)
    {
        // Update volume envelopes for channels 1, 2, 4
        if (_channel1.volumePeriod > 0)
        {
            _channel1.volumeTimer--;
            if (_channel1.volumeTimer <= 0)
            {
                _channel1.volumeTimer = _channel1.volumePeriod;
                if (_channel1.volumeIncrease && _channel1.volume < 15)
                {
                    _channel1.volume++;
                }
                else if (!_channel1.volumeIncrease && _channel1.volume > 0)
                {
                    _channel1.volume--;
                }
            }
        }
        
        if (_channel2.volumePeriod > 0)
        {
            _channel2.volumeTimer--;
            if (_channel2.volumeTimer <= 0)
            {
                _channel2.volumeTimer = _channel2.volumePeriod;
                if (_channel2.volumeIncrease && _channel2.volume < 15)
                {
                    _channel2.volume++;
                }
                else if (!_channel2.volumeIncrease && _channel2.volume > 0)
                {
                    _channel2.volume--;
                }
            }
        }
        
        if (_channel4.volumePeriod > 0)
        {
            _channel4.volumeTimer--;
            if (_channel4.volumeTimer <= 0)
            {
                _channel4.volumeTimer = _channel4.volumePeriod;
                if (_channel4.volumeIncrease && _channel4.volume < 15)
                {
                    _channel4.volume++;
                }
                else if (!_channel4.volumeIncrease && _channel4.volume > 0)
                {
                    _channel4.volume--;
                }
            }
        }
    }
    
    // Step 2, 6: Sweep (Channel 1 only)
    if (_frameSequencerStep == 2 || _frameSequencerStep == 6)
    {
        UpdateSweep();
    }
}

void Apu::UpdateChannel1()
{
    if (!_channel1.enabled || !_channel1.dacEnabled)
        return;
        
    // Update timer
    _channel1.timer--;
    if (_channel1.timer <= 0)
    {
        _channel1.timer = (2048 - _channel1.frequency) * 4;
        _channel1.dutyPosition = (_channel1.dutyPosition + 1) & 7;
    }
}

void Apu::UpdateChannel2()
{
    if (!_channel2.enabled || !_channel2.dacEnabled)
        return;
        
    // Update timer
    _channel2.timer--;
    if (_channel2.timer <= 0)
    {
        _channel2.timer = (2048 - _channel2.frequency) * 4;
        _channel2.dutyPosition = (_channel2.dutyPosition + 1) & 7;
    }
}

void Apu::UpdateChannel3()
{
    if (!_channel3.enabled || !_channel3.dacEnabled)
        return;
        
    // Update timer
    _channel3.timer--;
    if (_channel3.timer <= 0)
    {
        _channel3.timer = (2048 - _channel3.frequency) * 2;
        _channel3.position = (_channel3.position + 1) & 31;
    }
}

void Apu::UpdateChannel4()
{
    if (!_channel4.enabled || !_channel4.dacEnabled)
        return;
        
    // Update timer
    _channel4.timer--;
    if (_channel4.timer <= 0)
    {
        // Calculate divisor
        int divisor = _channel4.divisorCode == 0 ? 8 : _channel4.divisorCode * 16;
        _channel4.timer = divisor << _channel4.clockShift;
        
        // Update LFSR (Linear Feedback Shift Register)
        int xorResult = (_channel4.lfsr & 1) ^ ((_channel4.lfsr >> 1) & 1);
        _channel4.lfsr >>= 1;
        _channel4.lfsr |= xorResult << 14;
        
        if (_channel4.widthMode)
        {
            _channel4.lfsr &= ~(1 << 6);
            _channel4.lfsr |= xorResult << 6;
        }
    }
}

void Apu::GenerateSample()
{
    if (!_masterEnabled)
    {
        _audioBuffer.push_back(0.0f);
        _audioBuffer.push_back(0.0f);
        return;
    }
    
    // Get samples from each channel
    int ch1 = GetChannel1Sample();
    int ch2 = GetChannel2Sample();
    int ch3 = GetChannel3Sample();
    int ch4 = GetChannel4Sample();
    
    // Mix channels for left and right outputs
    int leftMix = 0;
    int rightMix = 0;
    
    if (_leftEnable & 0x01) leftMix += ch1;
    if (_leftEnable & 0x02) leftMix += ch2;
    if (_leftEnable & 0x04) leftMix += ch3;
    if (_leftEnable & 0x08) leftMix += ch4;
    
    if (_rightEnable & 0x01) rightMix += ch1;
    if (_rightEnable & 0x02) rightMix += ch2;
    if (_rightEnable & 0x04) rightMix += ch3;
    if (_rightEnable & 0x08) rightMix += ch4;
    
    // Apply master volume
    leftMix = (leftMix * (_leftVolume + 1)) / 8;
    rightMix = (rightMix * (_rightVolume + 1)) / 8;
    
    // Convert to float (-1.0 to 1.0)
    float leftSample = (leftMix - 32) / 32.0f;
    float rightSample = (rightMix - 32) / 32.0f;
    
    _audioBuffer.push_back(leftSample);
    _audioBuffer.push_back(rightSample);
    
    // Limit buffer size
    if (_audioBuffer.size() > BUFFER_SIZE * 2)
    {
        _audioBuffer.erase(_audioBuffer.begin(), _audioBuffer.begin() + 512);
    }
}

void Apu::InitializeRegisters()
{
    // Initialize APU registers to their default values
    // NR10-NR14: Channel 1 (0xFF10-0xFF14)
    _ioRegisters->Write(0xFF10, 0x80); // NR10 - Channel 1 Sweep
    _ioRegisters->Write(0xFF11, 0xBF); // NR11 - Channel 1 Length/Duty
    _ioRegisters->Write(0xFF12, 0xF3); // NR12 - Channel 1 Volume
    _ioRegisters->Write(0xFF13, 0x00); // NR13 - Channel 1 Frequency Low
    _ioRegisters->Write(0xFF14, 0xBF); // NR14 - Channel 1 Frequency High
    
    // NR20-NR24: Channel 2 (0xFF15-0xFF19)
    _ioRegisters->Write(0xFF16, 0x3F); // NR21 - Channel 2 Length/Duty
    _ioRegisters->Write(0xFF17, 0x00); // NR22 - Channel 2 Volume
    _ioRegisters->Write(0xFF18, 0x00); // NR23 - Channel 2 Frequency Low
    _ioRegisters->Write(0xFF19, 0xBF); // NR24 - Channel 2 Frequency High
    
    // NR30-NR34: Channel 3 (0xFF1A-0xFF1E)
    _ioRegisters->Write(0xFF1A, 0x7F); // NR30 - Channel 3 Enable
    _ioRegisters->Write(0xFF1B, 0xFF); // NR31 - Channel 3 Length
    _ioRegisters->Write(0xFF1C, 0x9F); // NR32 - Channel 3 Volume
    _ioRegisters->Write(0xFF1D, 0x00); // NR33 - Channel 3 Frequency Low
    _ioRegisters->Write(0xFF1E, 0xBF); // NR34 - Channel 3 Frequency High
    
    // NR40-NR44: Channel 4 (0xFF1F-0xFF23)
    _ioRegisters->Write(0xFF20, 0xFF); // NR41 - Channel 4 Length
    _ioRegisters->Write(0xFF21, 0x00); // NR42 - Channel 4 Volume
    _ioRegisters->Write(0xFF22, 0x00); // NR43 - Channel 4 Frequency
    _ioRegisters->Write(0xFF23, 0xBF); // NR44 - Channel 4 Control
    
    // NR50-NR52: Control registers (0xFF24-0xFF26)
    _ioRegisters->Write(0xFF24, 0x77); // NR50 - Master Volume
    _ioRegisters->Write(0xFF25, 0xF3); // NR51 - Sound Panning
    _ioRegisters->Write(0xFF26, 0xF1); // NR52 - Sound Enable (DMG: only bit 7 writable)
    
    // Wave pattern RAM (0xFF30-0xFF3F) - Initialize to zeros
    // Wave pattern RAM (0xFF30-0xFF3F) - Initialize to zeros
    for (word addr = 0xFF30; addr <= 0xFF3F; addr++)
    {
        _ioRegisters->Write(addr, 0x00);
    }
    
    // Internal wave pattern already initialized to zeros in Reset()
}

int Apu::GetChannel1Sample()
{
    if (!_channel1.enabled || !_channel1.dacEnabled)
        return 0;
        
    // Get duty cycle output
    int dutyOutput = DUTY_PATTERNS[_channel1.duty][_channel1.dutyPosition];
    return dutyOutput * _channel1.volume;
}

int Apu::GetChannel2Sample()
{
    if (!_channel2.enabled || !_channel2.dacEnabled)
        return 0;
        
    // Get duty cycle output
    int dutyOutput = DUTY_PATTERNS[_channel2.duty][_channel2.dutyPosition];
    return dutyOutput * _channel2.volume;
}

int Apu::GetChannel3Sample()
{
    if (!_channel3.enabled || !_channel3.dacEnabled)
        return 0;
        
    // Get wave sample
    byte sample = _channel3.wavePattern[_channel3.position];
    
    // Apply output level
    switch (_channel3.outputLevel)
    {
        case 0: return 0; // Mute
        case 1: return sample; // 100%
        case 2: return sample >> 1; // 50%
        case 3: return sample >> 2; // 25%
        default: return 0;
    }
}

int Apu::GetChannel4Sample()
{
    if (!_channel4.enabled || !_channel4.dacEnabled)
        return 0;
        
    // Get noise output (inverted LFSR bit 0)
    int noiseOutput = (~_channel4.lfsr) & 1;
    return noiseOutput * _channel4.volume;
}

void Apu::UpdateSweep()
{
    if (!_channel1.sweepEnabled || _channel1.sweepPeriod == 0)
        return;
        
    _channel1.sweepTimer--;
    if (_channel1.sweepTimer <= 0)
    {
        _channel1.sweepTimer = _channel1.sweepPeriod;
        
        int newFrequency = CalculateSweepFrequency();
        if (newFrequency <= 2047 && _channel1.sweepShift > 0)
        {
            _channel1.frequency = newFrequency;
            
            // Check overflow again
            if (CalculateSweepFrequency() > 2047)
            {
                _channel1.enabled = false;
            }
        }
    }
}

int Apu::CalculateSweepFrequency()
{
    int sweepAmount = _channel1.frequency >> _channel1.sweepShift;
    if (_channel1.sweepIncrease)
    {
        return _channel1.frequency + sweepAmount;
    }
    else
    {
        return _channel1.frequency - sweepAmount;
    }
}

void Apu::TriggerChannel1()
{
    _channel1.enabled = true;
    if (_channel1.lengthCounter == 0)
    {
        _channel1.lengthCounter = 64;
    }
    _channel1.timer = (2048 - _channel1.frequency) * 4;
    _channel1.volume = _channel1.volumeInitial;
    _channel1.volumeTimer = _channel1.volumePeriod;
    
    // Sweep initialization
    _channel1.sweepTimer = _channel1.sweepPeriod;
    _channel1.sweepEnabled = (_channel1.sweepPeriod > 0 || _channel1.sweepShift > 0);
    if (_channel1.sweepShift > 0)
    {
        if (CalculateSweepFrequency() > 2047)
        {
            _channel1.enabled = false;
        }
    }
}

void Apu::TriggerChannel2()
{
    _channel2.enabled = true;
    if (_channel2.lengthCounter == 0)
    {
        _channel2.lengthCounter = 64;
    }
    _channel2.timer = (2048 - _channel2.frequency) * 4;
    _channel2.volume = _channel2.volumeInitial;
    _channel2.volumeTimer = _channel2.volumePeriod;
}

void Apu::TriggerChannel3()
{
    _channel3.enabled = true;
    if (_channel3.lengthCounter == 0)
    {
        _channel3.lengthCounter = 256;
    }
    _channel3.timer = (2048 - _channel3.frequency) * 2;
    _channel3.position = 0;
}

void Apu::TriggerChannel4()
{
    _channel4.enabled = true;
    if (_channel4.lengthCounter == 0)
    {
        _channel4.lengthCounter = 64;
    }
    int divisor = _channel4.divisorCode == 0 ? 8 : _channel4.divisorCode * 16;
    _channel4.timer = divisor << _channel4.clockShift;
    _channel4.volume = _channel4.volumeInitial;
    _channel4.volumeTimer = _channel4.volumePeriod;
    _channel4.lfsr = 0x7FFF;
}