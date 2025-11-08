@echo off
REM Kiisu Production Release Build Script (Windows)
REM Optimization #10: Production build with LTO and aggressive optimization

setlocal enabledelayedexpansion

echo ========================================
echo Kiisu Production Release Build
echo ========================================
echo.

echo WARNING: Production build requirements:
echo   - 16GB+ RAM (LTO requires significant memory)
echo   - 8-15 minutes build time (2-3x longer than dev)
echo   - Ensure code is tested with dev profile first
echo.

set /p CONFIRM="Continue with production build? (y/N): "
if /i not "%CONFIRM%"=="y" (
    echo Build cancelled.
    exit /b 0
)

REM Set release profile
set BUILD_PROFILE=release

echo.
echo ========================================
echo Starting Production Build
echo ========================================
echo Profile: %BUILD_PROFILE%
echo Optimizations: LTO + Aggressive
echo Apps: All external apps included
echo.

REM Clean previous build artifacts
echo Cleaning previous build artifacts...
if exist "build\latest" rmdir /s /q "build\latest" 2>nul
if exist "dist\kiisu-prod" rmdir /s /q "dist\kiisu-prod" 2>nul

REM Build updater package
echo Building updater package...
echo.

set START_TIME=%time%

fbt.cmd updater_package

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo Build failed
    echo ========================================
    echo.
    echo Troubleshooting:
    echo 1. Check RAM usage (may have run out of memory during LTO)
    echo 2. Try dev profile first: fbt.cmd flash_usb_full
    echo 3. Review build errors above
    echo.
    exit /b 1
)

echo.
echo ========================================
echo Production build successful!
echo ========================================

REM Display firmware size
if exist "build\latest\firmware.bin" (
    for %%A in (build\latest\firmware.bin) do echo Firmware size: %%~zA bytes
)

echo.
echo ========================================
echo Release Artifacts
echo ========================================
echo Firmware:
if exist "build\latest" dir /b "build\latest\firmware.*" 2>nul
echo Distribution:
if exist "dist" dir /s /b "dist\*.tgz" "dist\*.dfu" 2>nul

echo.
echo ========================================
echo Next Steps
echo ========================================
echo 1. Test firmware on hardware:
echo    fbt.cmd flash_usb_full
echo.
echo 2. Deploy to qFlipper:
echo    Use package in dist\ folder
echo.
echo 3. Verify performance improvements:
echo    - Sensor polling should be smoother
echo    - UI responsiveness should be improved
echo    - Power consumption should be lower

echo.
echo Production build complete!

endlocal
