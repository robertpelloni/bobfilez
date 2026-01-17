@echo off
REM filez GUI Launcher
REM Launches CLI fat JAR

setlocal
set JAR_FILE=%~dp0filez-java\cli\build\libs\cli-2.1.0-all.jar
set MAIN_CLASS=com.filez.cli.FilezApp

REM Launch CLI in GUI-like console
java -Xmx256m -Xms128m -jar "%JAR_FILE%" scan --help

pause
