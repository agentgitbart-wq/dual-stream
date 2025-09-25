# DualStream Video Player

A sophisticated dual-video switching application built for fun and learning advanced C++ programming. This project demonstrates hardware-accelerated video processing, multiple rendering backends, and clean software architecture patterns.

## 🎯 Project Purpose

This application was developed as a learning exercise to explore:
- Advanced C++ programming techniques
- FFmpeg integration and video processing
- Hardware acceleration (NVIDIA NVDEC, DirectX, CUDA)
- Modern graphics APIs (DirectX 11, OpenGL 4.6)
- Clean architecture and design patterns
- Assisted development and tool support

## ✨ Features

### Video Processing
- **Multi-Resolution Support** - Videos don't need identical resolutions
- **Hardware Acceleration** - NVIDIA NVDEC with automatic software fallback
- **Codec Support** - H.264 and H.265 in MP4 containers
- **Advanced Switching** - Multiple algorithms with different performance characteristics

### Rendering System
- **Dual Backend Support** - DirectX 11 or OpenGL (compile-time selection)
- **Hardware Acceleration** - D3D11VA (DirectX) or CUDA interop (OpenGL)
- **YUV Rendering** - Direct YUV rendering with hardware color conversion

### User Interface
- **Dynamic Windows** - Resizable windows with renderer adjustment
- **Fullscreen Mode** - F11 toggle with state preservation
- **Multi-Monitor Support** - Window size limited by display resolution
- **Extensible Input** - Modular trigger framework

## 🚀 Quick Start

### Basic Usage
```bash
./dual_stream video1.mp4 video2.mp4
```

### Advanced Options
```bash
# Different switching algorithms
./dual_stream video1.mp4 video2.mp4 --algorithm predecoded
./dual_stream video1.mp4 video2.mp4 -a keyframe-sync

# Playback speed control
./dual_stream video1.mp4 video2.mp4 --speed 2.0

# Debug mode
./dual_stream video1.mp4 video2.mp4 --debug
```

### Controls
- **1/2 Keys** - Switch between videos
- **F11** - Toggle fullscreen
- **ESC** - Exit application

## ⚙️ Build Requirements

- **OS:** Windows 10/11 (64-bit)
- **Compiler:** Visual Studio 2019+ or MinGW-w64
- **CMake:** 3.15+
- **FFmpeg:** 7.1.1 (automatically downloaded by CMake)
- **GPU:** NVIDIA RTX series (recommended) or DirectX 11 compatible

### Build Instructions
```bash
mkdir build
cd build

# Both DirectX 11 and OpenGL renderers are always available
cmake ..

cmake --build . --config Release
```

## 🎮 Switching Algorithms

| Algorithm | Latency | Memory Usage | Quality | Use Case |
|-----------|---------|--------------|---------|----------|
| **Immediate** (Default) | ~1-5ms | 1x | Good | General purpose |
| **Predecoded** | ~0ms | 2x | Perfect | Zero-latency switching |
| **Keyframe Sync** | Variable | 1x | Perfect | Artifact-free transitions |

## 🏗️ Architecture

```
src/
├── core/          # Foundation services (CLI, logging, FFmpeg init)
├── ui/            # Window management and input handling
├── video/         # Video processing pipeline
│   ├── demux/     # Container parsing (MP4)
│   ├── decode/    # Hardware/software decoding
│   ├── switching/ # Video switching strategies
│   └── triggers/  # Input trigger system
└── rendering/     # Multi-backend rendering (D3D11/OpenGL)
```

The application uses modern C++ design patterns including:
- Strategy pattern for switching algorithms
- Factory pattern for renderer/trigger creation
- RAII for resource management
- Interface segregation for extensibility

## 📊 Performance

### Hardware Decoding Path (Optimal)
- Container Parsing: ~5% CPU
- Hardware Decoding: ~10% GPU (NVDEC)
- Format Conversion: ~5% GPU (shaders)
- Rendering: ~5% GPU (3D pipeline)

### Software Fallback Path
- Container Parsing: ~5% CPU  
- Software Decoding: ~40% CPU
- Format Conversion: ~20% CPU
- Rendering: ~5% GPU

## 📚 Documentation

Each component has detailed technical documentation:

- [Source Code Overview](src/ARCHITECTURE.md)
- [Core Services](src/core/ARCHITECTURE.md)
- [Video Processing](src/video/ARCHITECTURE.md)
- [Rendering System](src/rendering/ARCHITECTURE.md)
- [UI System](src/ui/ARCHITECTURE.md)

## 🎓 Learning Outcomes

This project provided hands-on experience with:

### C++ Advanced Concepts
- Modern C++17/20 features
- Memory management and RAII
- Template metaprogramming
- Design patterns implementation

### Multimedia Programming
- FFmpeg library integration
- Hardware-accelerated video decoding
- YUV color space handling
- Container format parsing

### Graphics Programming
- DirectX 11 API usage
- OpenGL 4.6 core profile
- CUDA-OpenGL interoperability
- Shader programming

### Software Architecture
- Clean architecture principles
- Dependency injection
- Strategy and factory patterns
- Extensible plugin systems

### Development Tools
- CMake build systems
- Cross-platform development
- AI-assisted programming
- Documentation-driven development

## 📝 License

This is a personal learning project created for educational purposes. Feel free to use it as a reference for your own multimedia programming journey!

---

*Built with curiosity* 🚀
