@echo off
setlocal
title BobFilez (CMake build)
cd /d "%~dp0"

echo [BobFilez (CMake build)] Starting...
where cmake >nul 2>nul
if errorlevel 1 (
    echo [BobFilez (CMake build)] cmake not found. Please install it.
    pause
    exit /b 1
)

cmake --build build --config Release

if errorlevel 1 (
    echo [BobFilez (CMake build)] Exited with error code %errorlevel%.
    pause
)
endlocal
