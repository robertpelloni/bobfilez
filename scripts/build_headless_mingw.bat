@echo off
REM build_headless_mingw: portable MinGW fallback when MSVC is absent (added 2026-11-03).
REM Prefers the portable C:\tools tree; falls back to cmake/ninja/gcc already
REM on PATH (e.g. w64devkit) so the lane works on hosts without C:\tools.
if exist "C:\tools\cmake\bin\cmake.exe" (
    set "PATH=C:\tools\cmake\bin;C:\tools\ninja;C:\tools\mingw64\bin;%PATH%"
)
where gcc >nul 2>nul || (echo [ERROR] no MinGW gcc found on PATH or at C:\tools\mingw64 & exit /b 1)
where cmake >nul 2>nul || (echo [ERROR] no cmake found on PATH or at C:\tools\cmake & exit /b 1)
where ninja >nul 2>nul || (echo [ERROR] no ninja found on PATH or at C:\tools\ninja & exit /b 1)
cd /d "%~dp0.."
cmake -S . -B build-mingw -G Ninja -DCMAKE_BUILD_TYPE=Release -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_TESTS=ON -DFO_BUILD_BENCH=OFF
if errorlevel 1 exit /b %errorlevel%
cmake --build build-mingw --parallel 2
