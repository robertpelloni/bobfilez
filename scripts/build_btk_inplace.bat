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

set "BTK_SRC=%~dp0..\libs\btk"
set "BTK_BUILD=%BTK_SRC%\build-btk"

echo [INFO] Using vcvars: %VCVARS%
call "%VCVARS%"
if errorlevel 1 exit /b %errorlevel%

cd /d "%BTK_SRC%"
if not exist "%BTK_BUILD%" mkdir "%BTK_BUILD%"
cd /d "%BTK_BUILD%"

echo [INFO] Configuring BTK in-place developer build...
cmake -S .. -B . -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
if errorlevel 1 exit /b %errorlevel%

echo [INFO] Building BTK in-place...
cmake --build . --parallel 2
exit /b %errorlevel%
