@echo off
REM dev_all: build headless, run tests, smoke CLI, verify dashboard wiring, start dashboard.
REM Heavy steps inherit the caller's priority; launch this file BelowNormal
REM (e.g. from a BelowNormal shell) on shared machines to avoid system hangs.
cd /d "%~dp0.."
call scripts\build_headless.bat || exit /b %errorlevel%
.\build-msvc\tests\fo_tests.exe || exit /b %errorlevel%
.\build-msvc\cli\fo_cli.exe --help || exit /b %errorlevel%
echo [dev_all] build+tests OK - verifying dashboard wiring...
cd bobui_web && node --test test/ || exit /b %errorlevel%
echo [dev_all] dashboard wiring OK - starting dashboard at http://localhost:3131/unified.html ...
node server.js
