@echo off
setlocal enabledelayedexpansion

REM Try different Visual Studio locations
set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo [ERROR] Could not find Visual Studio installation
    exit /b 1
)

echo [INFO] Found vcvars64 at: "%VCVARS%"
echo [INFO] Initializing environment...
call "%VCVARS%"

echo [INFO] Running CMake Configure for Fuzzers...
cmake -S . -B build_fuzz -G Ninja -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DFO_BUILD_GUI=OFF -DFO_BUILD_CLI=OFF -DFO_BUILD_BENCH=OFF -DFO_BUILD_TESTS=OFF -DFO_BUILD_FUZZERS=ON
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake Configure failed with code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo [INFO] Running CMake Build...
cmake --build build_fuzz
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake Build failed with code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Fuzzer build complete.
