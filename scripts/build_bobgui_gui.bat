@echo off
setlocal

set "BOBGUI_ROOT=%~dp0..\libs\bobgui"
set "BOBGUI_BUILD=%BOBGUI_ROOT%\_build"
set "APP_BUILD=%~dp0..\build-bobgui-app"

if exist "%~dp0build_headless.bat" (
    echo [INFO] Building bobfilez headless targets first so the optional fo_c_api library is available for BobGUI direct-mode detection...
    call "%~dp0build_headless.bat"
    if errorlevel 1 exit /b %errorlevel%
)

where meson >nul 2>&1
if errorlevel 1 (
    echo [ERROR] meson is required to build the BobGUI frontend
    exit /b 1
)

where ninja >nul 2>&1
if errorlevel 1 (
    echo [ERROR] ninja is required to build the BobGUI frontend
    exit /b 1
)

echo [INFO] Configuring BobGUI framework build...
if not exist "%BOBGUI_BUILD%\build.ninja" (
    meson setup "%BOBGUI_BUILD%" "%BOBGUI_ROOT%"
    if errorlevel 1 exit /b %errorlevel%
)

echo [INFO] Building BobGUI framework...
meson compile -C "%BOBGUI_BUILD%"
if errorlevel 1 exit /b %errorlevel%

set "PKG_CONFIG_PATH=%BOBGUI_BUILD%\meson-uninstalled;%PKG_CONFIG_PATH%"

echo [INFO] Configuring bobfilez BobGUI demo app...
if not exist "%APP_BUILD%\build.ninja" (
    meson setup "%APP_BUILD%" "%~dp0..\frontends\bobgui_app"
    if errorlevel 1 exit /b %errorlevel%
)

echo [INFO] Building bobfilez BobGUI demo app...
meson compile -C "%APP_BUILD%"
exit /b %errorlevel%
