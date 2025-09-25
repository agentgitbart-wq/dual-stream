# DualStream Video Player Application

A sophisticated dual-video switching application with hardware acceleration, multiple rendering backends, and advanced switching strategies.

## Project Status: ✅ **PRODUCTION READY**

The application has evolved far beyond its original requirements into a comprehensive, extensible video switching platform with advanced features and clean architecture.

## Core Features

### Video Processing
- **Multi-Resolution Support:** Videos no longer need identical resolutions
- **Hardware Acceleration:** NVIDIA NVDEC with automatic software fallback
- **Codec Support:** H.264 and H.265 in MP4 containers
- **Advanced Switching:** Multiple switching algorithms with different performance characteristics

### Rendering System  
- **Dual Backend Support:** DirectX 11 or OpenGL with runtime selection
- **Hardware Acceleration:** D3D11VA (DirectX) or CUDA interop (OpenGL)
- **Format Support:** Direct YUV rendering with hardware color conversion
- **Post-Processing Pipeline:** Comprehensive render pass system with multiple effects
- **ImGui Integration:** Seamless overlay rendering for debug UI and notifications

### User Interface
- **Advanced Window Management:** Resizable windows with dynamic renderer adjustment
- **Fullscreen Support:** F11 toggle with state preservation
- **ImGui Integration:** Comprehensive overlay system with debug UI and notifications
- **Global Input Handling:** Centralized input processing for UI and application hotkeys
- **Component Registry System:** Dynamic UI component registration and management
- **Toast Notifications:** Non-intrusive user feedback system
- **Camera Control UI:** Live camera preview and runtime property adjustment

### Advanced Configuration
- **Command Line Interface:** Rich parameter support for all features
- **Multiple Algorithms:** Immediate, predecoded, and keyframe-synchronized switching
- **Playback Speed Control:** Variable speed playback (0.1x to 10x)
- **Debug Logging:** Comprehensive logging with configurable verbosity
- **Post-Processing Pipeline:** Configurable render pass chain with multiple effects
- **Camera Integration:** Face detection-based automatic video switching
- **Camera Property Control:** Runtime adjustment of brightness, contrast, saturation, and gain
- **Live Camera Preview:** Real-time camera feed display with configurable refresh rate

## Architecture Overview

```
DualStream Video Player
├── src/                           # Complete source code
│   ├── main.cpp                   # Application orchestration
│   ├── core/                      # Foundation services
│   │   ├── CommandLineParser      # Advanced CLI processing
│   │   ├── Logger                 # Centralized logging system
│   │   └── FFmpegInitializer      # FFmpeg initialization
│   ├── ui/                        # User interface system
│   │   └── Window                 # Win32 window with modern features
│   ├── video/                     # Video processing pipeline
│   │   ├── VideoManager           # Central coordination
│   │   ├── VideoValidator         # File validation
│   │   ├── demux/                 # Container parsing
│   │   │   └── VideoDemuxer       # MP4 demuxing
│   │   ├── decode/                # Video decoding
│   │   │   ├── HardwareDecoder    # Hardware detection
│   │   │   └── VideoDecoder       # Multi-backend decoding
│   │   ├── switching/             # Switching strategies
│   │   │   ├── VideoSwitchingStrategy    # Strategy interface
│   │   │   ├── KeyframeSwitchStrategy    # Quality-focused switching
│   │   │   └── experimental/             # Alternative strategies
│   │   │       ├── ImmediateSwitchStrategy    # Default switching
│   │   │       └── PredecodedSwitchStrategy   # Zero-latency switching
│   │   └── triggers/              # Input handling
│   │       ├── ISwitchingTrigger  # Trigger interface
│   │       ├── KeyboardSwitchingTrigger      # Keyboard input
│   │       └── SwitchingTriggerFactory       # Trigger creation
│   ├── camera/                    # Camera system with computer vision
│   │   ├── CameraManager          # Central camera coordination
│   │   ├── sources/               # Camera source implementations
│   │   ├── processing/            # Frame processing and CV integration
│   │   └── ui/                    # Camera control UI components
│   │       ├── CameraControlUI    # ImGui-based camera control
│   │       └── CameraFrameTexture # Camera frame-to-texture conversion
│   └── rendering/                 # Multi-backend rendering
│       ├── IRenderer              # Renderer interface
│       ├── RendererFactory        # Renderer creation
│       ├── RenderTexture          # Generic texture abstraction
│       ├── TextureConverter       # Frame conversion
│       ├── D3D11Renderer          # DirectX 11 implementation
│       ├── OpenGLRenderer         # OpenGL 4.6 implementation
│       └── CudaOpenGLInterop      # CUDA-OpenGL interoperability
├── CMakeLists.txt                 # Build system with FFmpeg integration
├── test_videos/                   # Sample videos for testing
└── Documentation (ARCHITECTURE.md files in each directory)
```

## Usage

### Basic Usage
```bash
./dual_stream video1.mp4 video2.mp4
```

### Advanced Configuration  
```bash
# Switching algorithm selection
./dual_stream video1.mp4 video2.mp4 --algorithm predecoded
./dual_stream video1.mp4 video2.mp4 -a keyframe-sync

# Playback speed control
./dual_stream video1.mp4 video2.mp4 --speed 1.5
./dual_stream video1.mp4 video2.mp4 -s 0.5

# Debug logging
./dual_stream video1.mp4 video2.mp4 --debug

# Camera integration with face detection
./dual_stream video1.mp4 video2.mp4 --trigger=face-detection
./dual_stream video1.mp4 video2.mp4 --enable-camera

# Combined options
./dual_stream video1.mp4 video2.mp4 -a predecoded -s 2.0 -d
```

### Runtime Controls
- **1/2 Keys:** Switch between videos
- **F11:** Toggle fullscreen mode
- **F1:** Toggle debug UI overlay (includes camera controls)
- **ESC:** Exit application

### Camera Control UI
- **Live Preview:** Real-time camera feed display with configurable resolution
- **Property Adjustment:** Runtime control of brightness, contrast, saturation, and gain
- **Smart Ranges:** Automatic detection of camera capability ranges
- **Multi-Camera Support:** OpenCV and Intel RealSense camera sources

## Switching Algorithms

### Immediate (Default)
- **Behavior:** Seeks new video to current time and resumes
- **Latency:** ~1-5ms (seeking overhead)
- **Memory:** ~1x (single stream active)
- **Quality:** May have brief artifacts during transitions

### Predecoded  
- **Behavior:** Decodes both streams simultaneously for instant switching
- **Latency:** ~0ms (true zero-latency switching)
- **Memory:** ~2x (both streams active)
- **Quality:** Perfect frame synchronization

### Keyframe Sync
- **Behavior:** Waits for synchronized keyframes before switching
- **Latency:** Variable (0-2000ms depending on GOP size)
- **Memory:** ~1x (single stream active)  
- **Quality:** Perfect (no visual artifacts)

## Technical Specifications

### System Requirements
- **OS:** Windows 10/11 (64-bit)
- **GPU:** NVIDIA RTX series (recommended) or any DirectX 11 compatible
- **RAM:** 4GB+ (8GB+ for 4K videos with predecoded switching)
- **Storage:** SSD recommended for high bitrate videos

### Performance Characteristics
```
Hardware Decoding Path (Optimal):
├── Container Parsing: ~5% CPU
├── Hardware Decoding: ~10% GPU (NVDEC)
├── Format Conversion: ~5% GPU (shaders)
└── Rendering: ~5% GPU (3D pipeline)

Software Fallback Path:
├── Container Parsing: ~5% CPU
├── Software Decoding: ~40% CPU
├── Format Conversion: ~20% CPU
└── Rendering: ~5% GPU
```

### Memory Usage
```
Base Memory Usage:
├── Application: ~50MB
├── FFmpeg Libraries: ~20MB
├── Per Video Stream: ~10-100MB (resolution dependent)
└── GPU Textures: Video resolution dependent

Switching Strategy Impact:
├── Immediate/Keyframe: 1x base memory
└── Predecoded: 2x base memory (both streams active)
```

## Build Configuration

### Prerequisites
- **RealSense SDK:** Required dependency, install via vcpkg
- **vcpkg:** Package manager for C++ libraries
- **Visual Studio 2022:** For Windows builds

### Quick Start - Using Build Scripts

The project includes automated build scripts for easy development:

#### **Standard Build**
```bash
# On Windows, run batch files through cmd
cmd //c build.bat           # Release build with smart CMake detection
```

#### **Build Options**
```bash
cmd //c build.bat --debug   # Debug build with symbols
cmd //c build.bat --clean   # Clean build directory and rebuild
cmd //c build.bat --cmake   # Force CMake reconfiguration
```

#### **Clean Reset**
```bash
cmd //c clean.bat          # Remove all build artifacts
```

### Build Script Features
- **Smart CMake Detection:** Only runs CMake configuration when needed for fast incremental builds
- **Automatic vcpkg Integration:** Reads toolchain path from `.env.local` file
- **Build Performance:** ~10 seconds for incremental builds, ~3 minutes for clean builds
- **Cross-Configuration:** Supports both Release and Debug builds

### Manual Build (Alternative)
```cmake
# Configure with vcpkg toolchain (use your local vcpkg path)
# Note: Store your local vcpkg path in .env.local file (see .env.local for machine-specific configuration)
cmake -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake ..

# Build the project
cmake --build . --config Release
```

### Configuration Files
- **`.env.local`:** Contains machine-specific vcpkg toolchain path (gitignored)
  ```
  VCPKG_TOOLCHAIN_FILE=C:\Users\user\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake
  ```

### Renderer Selection
Both DirectX 11 and OpenGL renderers are now always compiled and available at runtime. No build-time configuration required.

### CUDA Support
- **Automatic Detection:** CMake detects CUDA toolkit availability
- **DirectX Mode:** Uses D3D11VA hardware acceleration
- **OpenGL Mode:** Uses CUDA-OpenGL interop for zero-copy rendering

## Documentation Structure

Each directory contains detailed technical documentation:

### Core Architecture
- **[src/ARCHITECTURE.md](src/ARCHITECTURE.md)** - Complete source code architecture overview
- **[src/core/ARCHITECTURE.md](src/core/ARCHITECTURE.md)** - Foundation services and utilities
- **[src/ui/ARCHITECTURE.md](src/ui/ARCHITECTURE.md)** - Comprehensive UI system with ImGui integration

### Video Processing
- **[src/video/ARCHITECTURE.md](src/video/ARCHITECTURE.md)** - Video processing system overview  
- **[src/video/demux/ARCHITECTURE.md](src/video/demux/ARCHITECTURE.md)** - Container parsing implementation
- **[src/video/decode/ARCHITECTURE.md](src/video/decode/ARCHITECTURE.md)** - Hardware decoding system
- **[src/video/switching/ARCHITECTURE.md](src/video/switching/ARCHITECTURE.md)** - Video switching strategies
- **[src/video/triggers/ARCHITECTURE.md](src/video/triggers/ARCHITECTURE.md)** - Input trigger system

### Camera System
- **[src/camera/ARCHITECTURE.md](src/camera/ARCHITECTURE.md)** - Complete camera system with computer vision integration
- **[src/camera/sources/ARCHITECTURE.md](src/camera/sources/ARCHITECTURE.md)** - Camera source abstraction and implementations
- **[src/camera/processing/ARCHITECTURE.md](src/camera/processing/ARCHITECTURE.md)** - Multi-threaded frame processing and CV integration
- **[src/camera/ui/ARCHITECTURE.md](src/camera/ui/ARCHITECTURE.md)** - Camera control UI system with live preview and property adjustment

### Rendering System
- **[src/rendering/ARCHITECTURE.md](src/rendering/ARCHITECTURE.md)** - Multi-backend rendering with comprehensive render pass system
- **[src/rendering/renderpass/ARCHITECTURE.md](src/rendering/renderpass/ARCHITECTURE.md)** - Render pass pipeline system architecture
- **[src/rendering/renderpass/d3d11/ARCHITECTURE.md](src/rendering/renderpass/d3d11/ARCHITECTURE.md)** - DirectX 11 render pass implementation
- **[src/rendering/renderpass/d3d11/passes/ARCHITECTURE.md](src/rendering/renderpass/d3d11/passes/ARCHITECTURE.md)** - DirectX 11 effect implementations
- **[src/rendering/renderpass/opengl/ARCHITECTURE.md](src/rendering/renderpass/opengl/ARCHITECTURE.md)** - OpenGL render pass implementation with CUDA interop
- **[src/rendering/renderpass/opengl/passes/ARCHITECTURE.md](src/rendering/renderpass/opengl/passes/ARCHITECTURE.md)** - OpenGL effect implementations

## Development Status

### ✅ Completed Features
- [x] CMake build system with FFmpeg 7.1.1 integration
- [x] Multi-backend rendering (DirectX 11 + OpenGL)
- [x] Hardware-accelerated decoding (NVDEC/D3D11VA/Software fallback)
- [x] Advanced video switching strategies (3 algorithms)
- [x] Extensible input trigger system
- [x] Multi-resolution video support
- [x] Dynamic window resizing and fullscreen mode
- [x] Variable playback speed control
- [x] Comprehensive error handling and logging
- [x] Complete documentation system
- [x] Camera control UI with live preview
- [x] Runtime camera property adjustment system
- [x] Normalized camera property values (0.0-1.0)
- [x] Multi-backend camera frame rendering (D3D11/OpenGL)

### 🚀 Architecture Highlights
- **Clean Separation:** Strategy patterns for switching algorithms and triggers
- **Extensible Design:** Easy addition of new strategies, triggers, and renderers
- **Performance Optimized:** Zero-copy hardware acceleration paths
- **Robust Error Handling:** Graceful degradation and comprehensive validation
- **Production Quality:** Memory management, resource cleanup, and error recovery

## License and Usage

This application demonstrates advanced video processing techniques and serves as a reference implementation for:
- Multi-backend hardware-accelerated video decoding
- Real-time video switching with multiple algorithms  
- Clean architecture patterns for multimedia applications
- Integration of FFmpeg with modern graphics APIs

The codebase provides a solid foundation for building sophisticated video processing applications with professional-grade performance and reliability.
