@echo off
cd /d "%~dp0"
if "%PORT%"=="" set PORT=3131
echo [bobfilez] dashboard at http://localhost:%PORT%/unified.html
node server.js
