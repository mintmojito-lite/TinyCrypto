@echo off
REM Run TinyCrypto bare-metal firmware in QEMU (raspi-pico = Cortex-M0+)
REM UART0 output goes directly to this terminal window.

set QEMU="C:\Program Files\qemu\qemu-system-arm.exe"
set ELF=build_arm\firmware.elf

if not exist %ELF% (
    echo ERROR: %ELF% not found. Run cmake --build build_arm first.
    exit /b 1
)

if not exist %QEMU% (
    echo ERROR: QEMU not found at %QEMU%.
    echo Install via: winget install -e --id SoftwareFreedomConservancy.QEMU
    exit /b 1
)

echo Launching QEMU with %ELF% ...
echo Press Ctrl+A then X to quit QEMU.
echo.
%QEMU% ^
    -machine raspi-pico ^
    -cpu cortex-m0plus ^
    -m 16M ^
    -nographic ^
    -kernel %ELF%
