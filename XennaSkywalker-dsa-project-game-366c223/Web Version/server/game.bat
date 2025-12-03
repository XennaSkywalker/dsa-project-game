@echo off
setlocal EnableDelayedExpansion
REM -------------------------------------------
REM  Game Server Build Script
REM -------------------------------------------

cd /d "%~dp0"

echo.
echo ==========================================
echo   Compiling Game Server...
echo ==========================================
echo.

REM Check if G++ is installed
where g++ >nul 2>nul
if !errorlevel! neq 0 (
    echo ERROR: G++ compiler not found!
    echo Please install MinGW-w64 (64-bit) and add it to your PATH.
    pause
    exit /b 1
)

REM Compile
REM -std=c++11: Standard C++
REM -lws2_32 -lwsock32: Links Windows Socket Libraries
REM -D_WIN32_WINNT=0x0A00: Sets Windows version to Win10 (Fixes WSAPoll/getaddrinfo errors)
REM -static: Prevents missing DLL errors

g++ main.cpp -o server.exe -std=c++11 -D_WIN32_WINNT=0x0A00 -static -lws2_32 -lwsock32

if !errorlevel! neq 0 (
    echo.
    echo ------------------------------------------
    echo   !!! COMPILATION FAILED !!!
    echo   Check the error messages above.
    echo   Ensure you are using a 64-bit MinGW compiler.
    echo ------------------------------------------
    pause
    exit /b 1
)

echo.
echo   Compilation Successful! Starting Server...
echo ==========================================
echo.

REM Open Browser
start "" "http://localhost:8080"

REM Run Server
server.exe

pause