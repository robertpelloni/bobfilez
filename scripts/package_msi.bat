@echo off
setlocal

echo [INFO] Checking for WiX Toolset...
where candle >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] WiX Toolset (candle.exe) not found in PATH.
    echo [ERROR] Please install WiX Toolset v3.11 from https://wixtoolset.org/releases/
    echo [ERROR] or add it to your PATH if already installed.
    echo [INFO] Attempting to continue in case it's in a standard path...
    
    if exist "C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe" (
        set "PATH=%PATH%;C:\Program Files (x86)\WiX Toolset v3.11\bin"
    )
)

where candle >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [FATAL] WiX Toolset still not found. Aborting.
    exit /b 1
)

if not exist "build\cli\fo_cli.exe" (
    echo [ERROR] fo_cli.exe not found. Please build the project first.
    exit /b 1
)

if not exist "wix\output" mkdir wix\output

echo [INFO] Compiling WiX project...
candle wix\main.wxs -out wix\output\main.wixobj -arch x64
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo [INFO] Linking MSI...
light wix\output\main.wixobj -out filez-2.1.0-win64.msi -b wix
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo [SUCCESS] MSI package created: filez-2.1.0-win64.msi
