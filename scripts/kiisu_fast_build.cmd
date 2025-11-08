@echo off
REM Kiisu Fast Development Build Script (Windows)
REM Optimization #11: Fast incremental builds for development

setlocal enabledelayedexpansion

echo ========================================
echo Kiisu Fast Development Build
echo ========================================
echo.

REM Set development profile
set BUILD_PROFILE=dev

REM Calculate parallel jobs (80% of processors)
set /a JOBS=%NUMBER_OF_PROCESSORS% * 4 / 5
if %JOBS% LSS 1 set JOBS=1

echo Using %JOBS% parallel jobs (%NUMBER_OF_PROCESSORS% cores available)
echo.

REM Determine build target (default: firmware only for speed)
set TARGET=%1
if "%TARGET%"=="" set TARGET=firmware

echo Building target: %TARGET%
echo.

REM Build
set START_TIME=%time%

fbt.cmd -j%JOBS% %TARGET%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo Build failed
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo Build successful
echo ========================================

REM Show output size if firmware was built
if exist "build\latest\firmware.bin" (
    for %%A in (build\latest\firmware.bin) do echo Firmware size: %%~zA bytes
)

echo.
echo Usage examples:
echo   %~nx0                    # Build firmware only (fastest)
echo   %~nx0 flash_usb_full     # Build and flash via USB
echo   %~nx0 fap_kiisu_sensor_hub  # Build single Kiisu app
echo   %~nx0 fap_deploy         # Build and deploy all Kiisu FAPs

endlocal
