@echo off
REM build_headless_mingw: portable MinGW fallback when MSVC is absent (added 2026-11-03).
REM Uses C:\tools\cmake, C:\tools\ninja, C:\tools\mingw64 (all portable, no admin).
set "TOOLS=C:\tools"
set "PATH=%TOOLS%\cmake\bin;%TOOLS%\ninja;%TOOLS%\mingw64\bin;%PATH%"
where gcc || (echo [ERROR] portable MinGW missing at %TOOLS%\mingw64 & exit /b 1)
cd /d "%~dp0.."
cmake -S . -B build-mingw -G Ninja -DCMAKE_BUILD_TYPE=Release -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_TESTS=ON -DFO_BUILD_BENCH=OFF
if errorlevel 1 exit /b %errorlevel%
cmake --build build-mingw --parallel 2