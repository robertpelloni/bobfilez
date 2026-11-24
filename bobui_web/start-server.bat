@echo off
cd /d "%~dp0"
if "%PORT%"=="" set PORT=3131
REM OCR needs the staged tessdata or it fails silently per request.
if "%TESSDATA_PREFIX%"=="" if exist "%~dp0..\build-lean-deps\x64-windows\share\tessdata" set "TESSDATA_PREFIX=%~dp0..\build-lean-deps\x64-windows\share\tessdata"
echo [bobfilez] dashboard at http://localhost:%PORT%/unified.html
node server.js
