@echo off
setlocal

set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo [ERROR] Could not find Visual Studio vcvars64.bat
    exit /b 1
)

set "BUILD_DIR=build-bobui-gui"
if not "%~1"=="" (
    set "BUILD_DIR=%~1"
)

if not "%~2"=="" (
    set "BOBUI_ROOT=%~2"
)

if "%BOBUI_ROOT%"=="" (
    set "BOBUI_ROOT=%~dp0..\libs\bobui\build-bobui"
)

echo [INFO] Using vcvars: %VCVARS%
call "%VCVARS%"
if errorlevel 1 exit /b %errorlevel%

cd /d "%~dp0.."

echo [INFO] BOBUI_ROOT=%BOBUI_ROOT%
echo [INFO] Configuring BobUI-backed GUI build into %BUILD_DIR%...
cmake -S . -B "%BUILD_DIR%" -G Ninja -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DFO_BUILD_GUI=ON -DFO_BUILD_OMNI=ON -DFO_BUILD_TESTS=ON -DFO_BUILD_BENCH=ON -DBOBUI_ROOT=%BOBUI_ROOT%
if errorlevel 1 exit /b %errorlevel%

echo [INFO] Building BobUI-backed GUI targets...
cmake --build "%BUILD_DIR%" --parallel 2
exit /b %errorlevel%
