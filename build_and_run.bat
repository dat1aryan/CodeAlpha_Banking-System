@echo off
echo.
echo  Compiling Nexus Bank - Banking System...
echo  ─────────────────────────────────────────
where g++ >nul 2>&1
if errorlevel 1 (
    echo  [!] g++ not found. Install MinGW-w64 or MSYS2.
    echo      https://www.mingw-w64.org/
    pause
    exit /b 1
)
g++ -std=c++17 -O2 -o banking_system.exe banking_system.cpp
if errorlevel 1 (
    echo.
    echo  [!] Compilation failed.
    pause
    exit /b 1
)
echo.
echo  [OK] banking_system.exe built successfully.
echo.
echo  Launching...
echo.
banking_system.exe
