# Ember Roadmap
This items in thi lst are not in any particular order. This list will be updated occasionally as development progresses.

## Future features
- Dialog API (ember_plat)
- More protocols for ember_net (TLS, QUIC)
- Legacy platform support
    - X11
    - DirectX11
    - OpenGL
    - PulseAudio

## Ember core
- [x] CMake build system
    - [x] Seperate modules into different targets
    - [x] Versioning
    - [x] Configuration flags
    - [ ] CMake test to flip features on/off
- [x] Semantic versioning across the library 
- [x] Error codes
- [ ] Documentation website (Doxygen)
- [x] Memory subsystem
- [x] Allocators
    - [ ] Scratch allocator
    - [x] Burst allocator
    - [x] `freelist` allocator
    - [x] `darray` object
    - [ ] Pool allocator
- [x] Subsystems
    - [x] `ember_platform` — windowing, input, time, and system info
    - [x] `ember_gpu` — low-level graphics API abstraction
    - [ ] `ember_audio` — audio abstraction
    - [ ] `ember_net` — networking utilities

## Supported platforms
- [ ] Windows 7 and later
    - [ ] Win32                     - windowing / input
    - [ ] XInput                    - gamepads
    - [ ] DirectX 12                - gpu interface
    - [ ] WASAPI                    - audio
    - [ ] Winsock                   - network

- [ ] macOS 11 and later
    - [ ] Cocoa                     - windowing / input
    - [ ] Game Controller Framework - gamepads
    - [ ] Metal4                    - gpu interface
    - [ ] Core Audio                - audio
    - [ ] POSIX                     - network

- [ ] Linux 
    - [ ] Wayland                   - windowing / input
    - [ ] XInput2                   - gamepads
    - [ ] Vulkan                    - gpu interface
    - [ ] ALSA                      - audio
    - [ ] POSIX                     - network

## ember_platform
- [x] Basic platform layer
- [x] Time subsystem 
    - [ ] Clock abstraction
    - [x] Current time
- [x] Threading utilities
- [ ] Input system
    - [ ] Send events for input
    - [ ] Desktop (keys / mouse)
    - [ ] Gamepad

## ember_gpu
- [x] Vtable-based abstraction
- [x] Render command structure
- [x] Export device capabilities from backend
- [x] Render format enum
- [ ] Mutil-window applications
- [x] Resource types
    - [x] `emgpu_rendertarget`
    - [x] `emgpu_pipeline`
    - [x] `emgpu_buffer`
    - [x] `emgpu_commandbuf`
    - [x] Frame abstraction instead of command buffers
- [x] Per-frame allocators (Scratch)
- [ ] Resource tracking across a frame
- [ ] Depth / stencil buffer
- [x] Vulkan backend
    - [ ] Optimise hot code (begin -> draw -> end)
    - [ ] Needs more validation
    - [x] Support offscreen rendering
- [ ] Sepereate GAPIs into diferrent modules (dll magic)
- [ ] Validation layers (like vulkan)
    - [ ] Standard validation
    - [ ] RenderDoc intergration