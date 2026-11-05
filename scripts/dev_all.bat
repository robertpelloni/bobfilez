@echo off
REM dev_all: build headless, run tests, smoke CLI, start dashboard.
cd /d "%~dp0.."
call scripts\build_headless.bat || exit /b %errorlevel%
.\build-msvc\tests\fo_tests.exe || exit /b %errorlevel%
.\build-msvc\cli\fo_cli.exe --help || exit /b %errorlevel%
echo [dev_all] build+tests OK - starting dashboard...
cd bobui_web && node server.js