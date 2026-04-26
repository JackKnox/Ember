# The Ember Toolbox

## Introduction
Ember is an open source toolbox for platform abstraction. It provides well-structured, regularly maintained modules for interfacing with most parts of your chosen operating system (see [Supported Platforms](#supported-platforms)). **Ember is written in pure C99 with a CMake build system.**

Ember is licensed under the [MIT License](LICENSE).

## Features
- CMake-based build system
- Absouluty zero dependecies! (except STL)
- Designed for low-level engine and application development
- Open source and easy to extend
- Modular design with optional subsystems:
    - **ember_platform** — windowing, input, time, and system info
    - **ember_gpu** — low-level graphics API abstraction
    - **ember_audio** — audio abstraction
    - **ember_net** — networking utilities

## System Requirements
- A C99-compatible compiler
- CMake ( >= 3.15 )
- [A supported platform toolchain](#supported-platforms)
- Git for cloning the repository

### Supported Platforms
- Windows 7 and later
- Linux (Wayland)
- **Coming Soon**
    - **macOS, X11 support, other POSIX-based systems.**

## Building
```bash
git clone https://github.com/JackKnox/Ember.git
cd Ember
cmake -S . -B build
cmake --build build