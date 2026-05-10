@echo off
REM Run the HomwWorkAssignment executable from the correct directory

cd /d "%~dp0"
.\out\build\x64-debug\HomwWorkAssignment.exe
pause
