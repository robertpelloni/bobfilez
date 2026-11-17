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
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo [ERROR] Could not find Visual Studio vcvars64.bat
    exit /b 1
)

echo [INFO] Using vcvars: %VCVARS%
call "%VCVARS%"
if errorlevel 1 exit /b %errorlevel%

cd /d "%~dp0.."

REM vcpkg's applocal step invokes PowerShell after linking executables. A stale
REM CMake cache can retain a removed preview path, so select an installed shell
REM explicitly instead of inheriting whatever an earlier configure discovered.
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

REM The vcpkg manifest intentionally stays lean. When the separately verified
REM provider prefix exists, supplement it so the manifest binary gets the same
REM OCR/OpenCV/ONNX providers as build-lean instead of silently lacking them.
set "LEAN_ARGS="
if exist "build-lean-deps\x64-windows\include\onnxruntime_cxx_api.h" (
    set "LEAN_ARGS=-DCMAKE_PREFIX_PATH=%CD%/build-lean-deps/x64-windows -DOpenCV_DIR=%CD%/build-lean-deps/opencv/x64/vc16/lib"
)

echo [INFO] Configuring headless build...
cmake -S . -B build-msvc -G Ninja -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_TESTS=ON -DFO_BUILD_BENCH=ON -DZ_VCPKG_PWSH_PATH="%PWSH%" -DZ_VCPKG_POWERSHELL_PATH="%PWSH%" %LEAN_ARGS%
if errorlevel 1 exit /b %errorlevel%

echo [INFO] Building headless targets...
cmake --build build-msvc --parallel 2
if errorlevel 1 exit /b %errorlevel%

REM CMake/vcpkg only sidecars DLLs from the manifest install tree. Copy the
REM supplemented lean-provider DLLs beside fo_cli.exe or Windows fails before
REM main with STATUS_DLL_NOT_FOUND when OCR/OpenCV/ONNX code is linked in.
if exist "build-lean-deps\x64-windows\bin\*.dll" copy /Y "build-lean-deps\x64-windows\bin\*.dll" "build-msvc\cli\" >nul
if exist "build-lean-deps\opencv\x64\vc16\bin\*.dll" copy /Y "build-lean-deps\opencv\x64\vc16\bin\*.dll" "build-msvc\cli\" >nul
exit /b 0
