# Ember Roadmap
This items in thi lst are not in any particular order. This list will be updated occasionally as development progresses.

## Ember core
- [x] CMake build system
    - [x] Seperate modules into different targets
    - [ ] (**ember_gpu**) Sepereate GAPIs into diferrent modules
    - [ ] Versioning
    - [ ] Configuration flags
    - [ ] CMake test to flip features on/off
- [ ] Semantic versioning across the library 
- [ ] Error codes
- [ ] Documentation website (Doxygen)
- [x] Math library (vector, math ops, etc.)
- [x] Memory subsystem
- [x] Allocators
    - [ ] Scratch allocator
    - [x] Burst allocator
    - [x] `freelist` allocator
    - [x] `darray` object
    - [] Pool allocator
- [x] Subsystems
    - [x] `ember_platform` — windowing, input, time, and system info
    - [x] `ember_gpu` — low-level graphics API abstraction
    - [ ] `ember_net` — networking utilities
    - [ ] `ember_audio` — audio abstraction

## ember_platform
- [x] Basic platform layer (Linux - X11)
    - [ ] Windows support
    - [ ] macOS support
    - [ ] Wayland support  
    - [ ] (Optional) mobile runtime support
- [x] Time subsystem 
    - [ ] Clock abstraction
    - [ ] Current time
- [x] Threading utilities
- [ ] Input system
    - [ ] Send events for input
    - [ ] Desktop (keys / mouse)
    - [ ] Gamepad

## ember_gpu
- [x] Vtable-based abstraction
- [x] Supported backends
    - [x] Vulkan backend
    - [ ] OpenGL backend
    - [ ] DX11/12 backend
    - [ ] Metal backend
- [x] Render command structure
- [x] Export device capabilities from backend
- [x] Render format enum
- [ ] Mutil-window applications
- [x] Resource types
    - [x] `emgpu_rendertarget`
    - [x] `emgpu_renderstage`
    - [x] `emgpu_renderbuffer`
    - [x] `emgpu_commandbuf`
    - [ ] Frame abstraction instead of command buffers
- [ ] Per-frame allocators (Scratch)
- [ ] Resource tracking across a frame
- [ ] Depth / stencil buffer
- [x] Vulkan backend
    - [ ] Optimise hot code (begin -> draw -> end)
    - [ ] Needs more validation
    - [ ] Support offscreen rendering
