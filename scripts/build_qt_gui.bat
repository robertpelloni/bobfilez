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

call "%VCVARS%"
if errorlevel 1 exit /b %errorlevel%

if "%QT6_ROOT%"=="" if not "%QT_ROOT%"=="" set "QT6_ROOT=%QT_ROOT%"
if "%QT6_ROOT%"=="" if not "%QTDIR%"=="" set "QT6_ROOT=%QTDIR%"

if "%QT6_ROOT%"=="" if exist "D:\Qt\6.11.0\mingw_64\lib\cmake\Qt6\Qt6Config.cmake" (
    set "QT6_ROOT=D:\Qt\6.11.0\mingw_64"
    echo [INFO] Auto-detected QT6_ROOT=%QT6_ROOT%
)

if "%QT6_ROOT%"=="" (
    echo [WARN] QT6_ROOT not set. Point QT6_ROOT or QTDIR at a compatible Qt6 desktop kit before building the pure Qt demo.
) else (
    echo [INFO] Using QT6_ROOT=%QT6_ROOT%
    echo %QT6_ROOT% | findstr /I "mingw" >nul
    if not errorlevel 1 (
        echo [WARN] The detected Qt kit appears to be MinGW-based while this helper currently initializes an MSVC environment. Pure Qt demo configuration is now intentionally separated from BobUI package hints, so a native MSVC desktop Qt kit is still the cleanest validation path.
    )
)

cmake -S "%~dp0.." -B "%~dp0..\build-qt-demo" -G Ninja -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_PREFIX_PATH="%QT6_ROOT%" -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_QT_DEMO=ON
if errorlevel 1 exit /b %errorlevel%

cmake --build "%~dp0..\build-qt-demo" --parallel 2
exit /b %errorlevel%
