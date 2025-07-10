#pragma once

#include "Core/Definitions.h"

// Forward declarations
class VRam;
class IoRegisters;

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
    
    virtual void RenderBackground(const VRam& vRam, const IoRegisters& ioRegisters) = 0;
};