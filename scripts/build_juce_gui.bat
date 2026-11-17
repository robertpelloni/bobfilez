@echo off
setlocal

set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat" (
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

REM vcpkg applocal needs an installed PowerShell; do not trust a stale
REM cached preview path (same fix as build_headless.bat).
set "PWSH="
if exist "C:\Program Files\PowerShell\7\pwsh.exe" (
    set "PWSH=C:\Program Files\PowerShell\7\pwsh.exe"
) else if exist "C:\Program Files\PowerShell\7-preview\pwsh.exe" (
    set "PWSH=C:\Program Files\PowerShell\7-preview\pwsh.exe"
) else if exist "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" (
    set "PWSH=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
)
if "%PWSH%"=="" (
    echo [ERROR] Could not find pwsh.exe or Windows PowerShell
    exit /b 1
)

cmake -S "%~dp0.." -B "%~dp0..\build-juce-demo" -G Ninja -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_JUCE_DEMO=ON -DZ_VCPKG_PWSH_PATH="%PWSH%" -DZ_VCPKG_POWERSHELL_PATH="%PWSH%"
if errorlevel 1 exit /b %errorlevel%

cmake --build "%~dp0..\build-juce-demo" --parallel 2
exit /b %errorlevel%
