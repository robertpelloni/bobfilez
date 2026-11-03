@echo off
setlocal enabledelayedexpansion

:: Initialize Visual Studio Environment
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo WARNING: VCVars64.bat not found. Assuming environment is already set up.
)

:: Configure with Fuzzing enabled
echo [1/2] Configuring CMake...
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="vcpkg/scripts/buildsystems/vcpkg.cmake" -DFO_BUILD_FUZZ_TESTS=ON
if errorlevel 1 exit /b 1

:: Build Fuzz Targets
echo [2/2] Building Fuzz Targets...
cmake --build build --target fuzz_metadata fuzz_rule_engine
if errorlevel 1 exit /b 1

echo Fuzz targets built successfully.
