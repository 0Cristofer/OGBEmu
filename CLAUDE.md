# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This project uses Premake5 for project generation and Visual Studio for building.

**Generate project files:**
```bash
cmd.exe /c GenerateProjects.bat
```

**Build configurations:**
- Debug: `HZ_DEBUG` defined, symbols enabled
- Release: `HZ_RELEASE` defined, optimizations enabled
- Dist: `HZ_DIST` defined, optimizations enabled

**Build from command line (after generating):**
```bash
# Build all configurations (use full path on WSL)
MSBuild.exe OGBEmu.sln

# Build specific configuration
MSBuild.exe OGBEmu.sln /p:Configuration=Debug
MSBuild.exe OGBEmu.sln /p:Configuration=Release
```

## Architecture Overview

This is a Game Boy emulator written in C++20 with a component-based architecture:

### Core Components

- **Device** (`Device.h/cpp`): Main emulator orchestrator that owns all components and manages the emulation loop
- **CPU** (`Cpu.h/cpp`): Game Boy CPU implementation with instruction execution
- **Bus** (`Memory/Bus.h/cpp`): Memory bus that routes read/write operations to appropriate memory regions
- **Memory Components**: Each memory region is a separate class (VRam, WRam, etc.)

### Memory Architecture

The memory system follows Game Boy's memory map with dedicated classes for each region:

- **BootRom**: Boot ROM handling
- **Cartridge**: Game cartridge with MBC (Memory Bank Controller) support
- **VRam**: Video RAM 
- **WRam/WRamCgb**: Work RAM (standard and CGB variants)
- **EchoRam**: Echo RAM region
- **Oam**: Object Attribute Memory
- **IoRegisters**: I/O registers
- **HRam**: High RAM

### MBC (Memory Bank Controller) System

- **BaseMbc**: Abstract base class for all MBC implementations
- **NoMbc**: For cartridges without bank switching
- **Mbc1**: MBC1 implementation
- Extensible design for adding more MBC types

### Key Design Patterns

1. **Composition over Inheritance**: Device owns all components rather than inheriting
2. **Pointer-based Dependencies**: Bus receives pointers to memory components for loose coupling
3. **Interface Segregation**: Each memory region has specific read/write methods
4. **Virtual Interface**: MBC system uses virtual methods for different cartridge types

## Development Notes

- C++20 standard is used throughout
- All source files are in `OGBEmu/src/`
- Core utilities and definitions are in `Core/`
- Emulator-specific code is in `Emulator/`
- Memory components are organized in `Emulator/Memory/`
- Custom types defined in `Core/Definitions.h` (byte, word, etc.)

## Game Boy Technical Documentation

**Primary Reference:** https://gbdev.io/pandocs/Specifications.html

**Boot ROM Assembly Reference:** https://www.neviksti.com/DMG/DMG_ROM.asm

**Key Specifications:**
- Screen: 160 × 144 pixels
- Sprites: 8 × 8 or 8 × 16 pixels (max 40 per screen, 10 per line)
- Palettes: Background 1 × 4 colors, Sprites 2 × 3 colors
- Boot ROM initializes LCDC register to 0x91 (LCD on, BG on, sprites off)
- Boot ROM writes 0x01 to 0xFF50 to disable itself at completion

## Recent Fixes and Improvements

### Stack Corruption Bug Fixes (Fixed)

**Issue**: Stack pointer corruption to prohibited memory range (0xFEA0-0xFEFF).

**Root Cause**: Multiple bugs in the `Add16` function and opcode routing:

1. **Add16 function bug** (`Cpu.cpp:890-894`): Function was incrementing the source parameter instead of adding source to HL register
2. **Opcode routing bug** (`Cpu.cpp:494`): ADD HL,SP (0x39) was checking wrong high nibble (0x4 instead of 0x3)
3. **Missing flag handling**: Add16 wasn't setting proper flags (N, H, C)

**Fixes Applied**:
- Fixed `Add16` to properly add source to HL register with correct flag handling
- Fixed opcode routing for 0x39 to check high nibble 0x3
- Added proper Game Boy flag handling: N=0, H=half-carry from bit 11, C=carry from bit 15
- Corrected cycle timing to 4 cycles (8 total with fetch)

### Echo RAM Implementation (Fixed)

**Issue**: Echo RAM had incorrect implementation with separate storage instead of true mirroring.

**Previous Implementation**:
- EchoRam maintained separate `_bytes` vector
- Manual mirroring in WriteWRam/WriteCgbWRam
- Inconsistent read/write behavior

**Fixed Implementation** (`Bus.cpp`):
- `ReadEchoRam`: Now directly calls `ReadWRam` with calculated address
- `WriteEchoRam`: Now directly calls `WriteWRam` with calculated address  
- Removed manual mirroring logic from WriteWRam/WriteCgbWRam
- True hardware-accurate mirroring behavior

### Logging System Improvements

**File Logging** (`Logger.cpp`):
- Added file output to `emulator_log.txt` (created relative to executable location)
- Clean start each session (overwrites previous log)
- Includes timestamps with millisecond precision for each log entry
- Session header shows when logging started to distinguish between runs
- Automatic fallback to console if file can't be opened
- Log file is excluded from git tracking (added to .gitignore)

**Logging Performance Optimization** (`Cpu.cpp`):
- Removed verbose stack operation logging that was causing performance issues
- Stack operation logs (PUSH, POP, CALL, RET, RST) were generating 300k+ lines per run
- Now only logs critical errors and debug breakpoints
- Significant performance improvement for emulation speed

### Memory Map Notes

**Echo RAM (0xE000-0xFDFF)**:
- Nintendo prohibits usage but hardware allows it
- Can cause short circuits with cartridge RAM on real hardware
- mooneye-gb tests intentionally use this for edge case testing
- SP=0xE000 is technically valid but not recommended

**Valid Stack Ranges**:
- 0xC000-0xDFFF: Work RAM (recommended)
- 0xFF80-0xFFFE: High RAM (traditional location)
- 0xE000-0xFDFF: Echo RAM (discouraged but functional)

### Verified Correct Implementations

**RST Instruction** (`Cpu.cpp:1023-1026`):
- Correctly identifies RST opcodes (0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF)
- Proper address calculation: `(opcode.row5 - 030) * 0x8`
- Correct behavior: CALL to fixed addresses (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38)

**ReadImm16AtPc Function** (`Cpu.cpp:222-228`):
- Correctly reads 16-bit immediate values in little-endian format
- Proper byte order: low byte first, then high byte shifted left

### Build System Notes

**Remember to recompile tests after any code changes** - Tests project must be rebuilt when core emulator code changes.

**Always check exit codes** - Use `; echo "Exit code: $?"` after running tests or executables to verify success (0) or failure (non-zero).

**Execute build and run commands directly** - No need to ask permission for standard build, test, or execution commands.

**Use git.exe instead of just git** - In WSL environment, use `git.exe` for all git commands to ensure proper Windows git integration.

**Always check log timestamps before analyzing** - Always verify the timestamp of log files to ensure you're analyzing the most recent execution, unless stated otherwise.