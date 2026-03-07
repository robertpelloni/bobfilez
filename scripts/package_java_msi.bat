@echo off
setlocal

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

if not exist "filez-java\cli\build\libs\cli-2.1.0-all.jar" (
    echo [ERROR] cli-2.1.0-all.jar not found. Please build the filez-java project first.
    exit /b 1
)

if not exist "wix\output" mkdir wix\output

echo [INFO] Compiling WiX project for Java Port...
candle wix\filez_java.wxs -out wix\output\filez_java.wixobj -arch x64
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo [INFO] Linking MSI...
light wix\output\filez_java.wixobj -out filez-java-2.1.0-win64.msi -b wix
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

echo [SUCCESS] MSI package created: filez-java-2.1.0-win64.msi
