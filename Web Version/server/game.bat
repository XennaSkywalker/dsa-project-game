@echo off
REM -------------------------------
REM Compile main.cpp to server.exe
REM -------------------------------

REM Change directory to where main.cpp is located
cd /d "%~dp0"

echo Compiling main.cpp...
g++ main.cpp Player.h GameState.h SaveManager.h ReplayManager.h -o server.exe -std=c++17 -lws2_32
IF ERRORLEVEL 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!
echo Running server...
server.exe

pause
