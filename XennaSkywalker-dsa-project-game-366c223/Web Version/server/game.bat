@echo off
setlocal EnableDelayedExpansion
REM -------------------------------------------
REM  Game Server Build Script
REM -------------------------------------------

echo.
echo ==========================================
echo   Compiling Game Server...
echo ==========================================
echo.


REM Compile
REM -std=c++11: Standard C++
REM -lws2_32 -lwsock32: Links Windows Socket Libraries
REM -D_WIN32_WINNT=0x0A00: Sets Windows version to Win10 (Fixes WSAPoll/getaddrinfo errors)
REM -static: Prevents missing DLL errors
g++ main.cpp Player.h GameState.h SaveManager.h ReplayManager.h DecisionTree.h TutorialManager.h -o server.exe -std=c++17 -D_WIN32_WINNT=0x0A00 -static -lws2_32 -lwsock32


echo.
echo   Compilation Successful! Starting Server...
echo ==========================================
echo.

REM Open Browser
start "" "http://localhost:8080"

REM Run Server
server.exe

pause