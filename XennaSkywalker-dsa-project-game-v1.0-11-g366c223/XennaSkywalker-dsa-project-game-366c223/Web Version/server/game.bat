@echo off
REM -------------------------------------------
REM  Game Server Build Script
REM -------------------------------------------

cd /d "%~dp0"

echo.
echo ==========================================
echo   Compiling Game Server...
echo ==========================================
echo.

REM Note: We only compile main.cpp. The .h files are included automatically.
REM We force disable C++17 features to fix the "optional" error.
g++ main.cpp Player.h GameState.h SaveManager.h ReplayManager.h DecisionTree.h TutorialManager.h -o server.exe -std=c++17 -lws2_32

IF ERRORLEVEL 1 (
    echo.
    echo ------------------------------------------
    echo   !!! COMPILATION FAILED !!!
    echo   Check the errors above.
    echo ------------------------------------------
    pause
    exit /b 1
)

echo.
echo   Compilation Successful! Starting Server...
echo ==========================================
echo.

REM Start server and wait a moment
start "Game Server" cmd /k server.exe
timeout /t 2 /nobreak >nul

REM Open browser
start "" "http://localhost:8080"

exit