@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cmake --build build --parallel 1
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
.\build\tests\fo_tests.exe --gtest_filter=StdLinterTest.*
