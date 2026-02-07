@echo off
setlocal

rem Check for Visual Studio environment
if not defined VCINSTALLDIR (
    echo Error: Visual Studio environment not set.
    echo Please run this script from the "x64 Native Tools Command Prompt" for VS 2019/2022.
    exit /b 1
)

if not defined JAVA_HOME (
    echo Error: JAVA_HOME environment variable not set.
    exit /b 1
)

set BLAKE3_DIR=..\..\..\libs\BLAKE3\c
set XXHASH_DIR=..\..\..\libs\xxHash

if not exist "%BLAKE3_DIR%" (
    echo Error: BLAKE3 directory not found at %BLAKE3_DIR%
    echo Did you run 'git submodule update --init'?
    exit /b 1
)

if not exist "%XXHASH_DIR%" (
    echo Error: xxHash directory not found at %XXHASH_DIR%
    exit /b 1
)

echo Compiling filez_native.dll...

cl /LD /O2 /I"%JAVA_HOME%\include" /I"%JAVA_HOME%\include\win32" ^
   /I"%BLAKE3_DIR%" /I"%XXHASH_DIR%" ^
   src\main\c\filez_native.c "%BLAKE3_DIR%\blake3.c" "%BLAKE3_DIR%\blake3_dispatch.c" "%BLAKE3_DIR%\blake3_portable.c" "%BLAKE3_DIR%\blake3_sse2.c" "%BLAKE3_DIR%\blake3_sse41.c" "%BLAKE3_DIR%\blake3_avx2.c" "%BLAKE3_DIR%\blake3_avx512.c" "%XXHASH_DIR%\xxhash.c" ^
   /Fe:filez_native.dll

if %ERRORLEVEL% EQU 0 (
    echo Build successful: filez_native.dll
    
    rem Move to resources
    mkdir src\main\resources\native\win32-x86-64 2>nul
    copy /Y filez_native.dll src\main\resources\native\win32-x86-64\
    echo Copied to resources.
) else (
    echo Build failed.
)

endlocal
