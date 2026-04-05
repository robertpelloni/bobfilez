@echo off
setlocal

set "BTK_ROOT=%~1"
if "%BTK_ROOT%"=="" set "BTK_ROOT=%~dp0..\libs\btk"

python "%~dp0prepare_btk_buildtree_package.py" "%BTK_ROOT%"
exit /b %errorlevel%
