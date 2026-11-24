@echo off
setlocal

REM Version truth is VERSION.md (single source; never hardcode here).
set /p VERSION=<VERSION.md
set VERSION=%VERSION: =%

echo [INFO] Checking for WiX Toolset...
where candle >nul 2>nul
if %ERRORLEVEL% EQU 0 goto :wix_found

echo [ERROR] WiX Toolset (candle.exe) not found in PATH.
echo [ERROR] Please install WiX Toolset v3.11 from https://wixtoolset.org/releases/
echo [ERROR] or add it to your PATH if already installed.
echo [INFO] Attempting to continue in case it's in a standard path...

if exist "C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe" (
    set "PATH=%PATH%;C:\Program Files (x86)\WiX Toolset v3.11\bin"
)

:wix_found

where candle >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [FATAL] WiX Toolset still not found. Aborting.
    exit /b 1
)

REM Find a built CLI in any current lane (newest layout first).
set "CLI="
for %%D in (build-msvc build-lean build-juce-demo build) do (
    if not defined CLI if exist "%%D\cli\fo_cli.exe" set "CLI=%CD%\%%D\cli\fo_cli.exe"
)
if not defined CLI (
    echo [ERROR] fo_cli.exe not found in build-msvc/build-lean/build-juce-demo/build. Please build the project first.
    exit /b 1
)
echo [INFO] Packaging %CLI% as version %VERSION%

if not exist "wix\output" mkdir wix\output

echo [INFO] Compiling WiX project...
candle wix\main.wxs -out wix\output\main.wixobj -arch x64 -dProductVersion=%VERSION% -dCliSource="%CLI%"
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo [INFO] Linking MSI...
light wix\output\main.wixobj -out filez-%VERSION%-win64.msi -b wix
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo [SUCCESS] MSI package created: filez-%VERSION%-win64.msi
