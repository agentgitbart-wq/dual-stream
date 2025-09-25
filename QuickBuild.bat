@echo off
REM Quick Build Script for DualStream Video Player
REM Enhanced for CenterFace face detection integration

setlocal EnableDelayedExpansion

echo ========================================
echo DualStream Quick Build Script
echo ========================================

REM Check if .env.local exists
if not exist ".env.local" (
    echo ERROR: .env.local file not found!
    echo Creating default .env.local...
    echo VCPKG_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake > .env.local
    echo Please edit .env.local with your correct vcpkg path
    pause
    exit /b 1
)

REM Set default configuration
set BUILD_CONFIG=Release
set ENABLE_FACE_DETECTION=ON

REM Parse simple arguments
if /i "%1"=="debug" set BUILD_CONFIG=Debug
if /i "%1"=="clean" (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
    echo Clean completed.
)

echo Quick building: %BUILD_CONFIG% configuration
echo Face detection: %ENABLE_FACE_DETECTION%
echo.

REM Read vcpkg toolchain path
for /f "tokens=2 delims==" %%a in ('findstr "VCPKG_TOOLCHAIN_FILE" .env.local') do set VCPKG_TOOLCHAIN=%%a

if "%VCPKG_TOOLCHAIN%"=="" (
    echo ERROR: VCPKG_TOOLCHAIN_FILE not configured properly
    exit /b 1
)

REM Create build directory
if not exist build mkdir build

REM Quick CMake configuration
cd build
echo Configuring CMake for CenterFace integration...
cmake -G "Visual Studio 17 2022" ^
      -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%" ^
      -DENABLE_CAMERA_SUPPORT=ON ^
      -DDOWNLOAD_FACE_DETECTION_MODELS=ON ^
      -DUSE_OPENGL_RENDERER=OFF ^
      ..

if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo Building project (target: dual_stream only)...
cmake --build . --config %BUILD_CONFIG% --target dual_stream --parallel

if errorlevel 1 (
    echo ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

cd ..

echo ========================================
echo Build completed successfully!
echo ========================================
echo Executable: build\bin\%BUILD_CONFIG%\dual_stream.exe
echo.
echo FEATURES ENABLED:
echo ✓ CenterFace ONNX face detection (default)
echo ✓ YuNet ONNX face detection (fallback)  
echo ✓ Haar Cascade face detection (reliable fallback)
echo ✓ Intel RealSense depth camera support
echo ✓ Multi-model automatic fallback
echo ✓ Distance-based intelligent switching
echo ✓ Optical parameters calculation
echo.
echo QUICK TEST:
echo   build\bin\%BUILD_CONFIG%\dual_stream.exe --help
echo.
echo USAGE WITH FACE DETECTION:
echo   build\bin\%BUILD_CONFIG%\dual_stream.exe video1.mp4 video2.mp4
echo   # Face detection will automatically control video switching
echo ========================================

REM Optional: Start the application if videos exist
if exist "test_videos\video1.mp4" if exist "test_videos\video2.mp4" (
    echo.
    echo Test videos found! Press any key to start demo...
    pause >nul
    build\bin\%BUILD_CONFIG%\dual_stream.exe test_videos\video1.mp4 test_videos\video2.mp4
)

pause