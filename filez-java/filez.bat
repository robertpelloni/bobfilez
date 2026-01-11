@echo off
setlocal

set SCRIPT_DIR=%~dp0
set CP=%SCRIPT_DIR%build\classes
set CP=%CP%;%SCRIPT_DIR%picocli-4.7.5.jar
set CP=%CP%;%SCRIPT_DIR%sqlite-jdbc-3.45.1.0.jar
set CP=%CP%;%SCRIPT_DIR%slf4j-api-2.0.9.jar
set CP=%CP%;%SCRIPT_DIR%slf4j-simple-2.0.9.jar

java -Xmx256m -Xms64m -cp "%CP%" com.filez.cli.FilezApp %*
