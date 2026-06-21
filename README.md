# Ember 🔥

> One Protocol. Multiple Drivers. Unlimited Possibilities.

Ember is a modern, cross-platform systems protocol for applications, engines, tools, and runtimes. Write your application once and deploy it everywhere — desktop, mobile, embedded, web, console, server.

---

## What is Ember?

Most software stacks eventually become tightly coupled to a specific platform or implementation. An application written directly against native APIs must carry the complexity of every platform it targets.

Ember solves this with a clean separation of concerns:

- **The Protocol** defines the contract: API shapes, data formats, object lifetimes, synchronization rules, and extension mechanisms.
- **Drivers** implement that contract for a specific platform.
- **Extensions** allow for innovation without changing the contract.
- **Your application** targets the Protocol — never a Driver directly.

This means your application stays portable while Drivers evolve independently to support new platforms, APIs, and hardware capabilities.

```
Application
     │
     ▼
┌──────────────────┐
│  Ember Protocol  │────────┐
└──────────────────┘        ▼
     │                 ┌──────────────────┐
     │                 │    Extensions    │
     ▼                 └──────────────────┘
┌──────────────────┐        |
│     Driver       │◀──────┘
└──────────────────┘
     │
     ├─ Win32 / Wayland / Cocoa
     ├─ DirectX 12 / Vulkan / Metal
     ├─ WASAPI / ALSA / Pipewire
     └─ Winsock / BSD Sockets
```

---

## Drivers

A Driver is any implementation of the Ember Protocol. Drivers may target:

- Desktop platforms
- Mobile platforms
- Embedded systems
- Websites (WASM)
- Consoles
- Headless servers
- Custom environments

As long as a Driver conforms to the protocol, your application is compatible with it — no changes required.

### Official Driver

The primary reference implementation is **`ember-native`**, targeting desktop, mobile, and web. See the [official repository](https://github.com/JackKnox/ember-native) to get started.

---

## Architecture

The protocol is organized into three layers:

```
Core
 ├─ Domains
 │   ├─ Platform
 │   ├─ Window
 │   ├─ GPU
 │   ├─ Audio
 │   └─ Network
 └─ Extensions
```

### Core

Core is the foundation shared by every compliant Driver. All implementations must conform to the Core specification.

| Feature | Description |
|---------|-------------|
| **Memory**            | Allocation APIs, tracking, statistics, custom allocators.     |
| **Logging**           | Structured logging, sinks, hooks, runtime filtering.          |
| **Error Handling**    | Standardized result codes, propagation, diagnostics.          |
| **Datastreams**       | Unified stream abstraction — memory, file, and custom.        |
| **Format API**        | Serialization helpers, encoding utilities, runtime discovery. |

### Domains

Domains define the major capabilities every Driver must implement. Every compliant Driver provides every Domain.

| Domain | Responsibility |
|--------|----------------|
| **Platform**            | OS integration, process lifecycle, system events |
| **Window**              | Window creation, input handling, monitor control |
| **GPU**                 | Graphics and compute pipeline                    |
| **Audio**               | Playback, capture                                |
| **Network**             | Socket communication, connection management      |

### Extensions

The core protocol is intentionally small. Advanced functionality is exposed through Extensions, allowing Drivers and vendors to innovate without fragmenting the protocol.

| Type | Owner | Example |
|------|-------|---------|
| **Core Extensions**    | Ember Protocol      | `EMGPU_EXT_mesh_shader`    |
| **Driver Extensions**  | Specific Driver     | `EMGPU_DSK_pipeline_cache` |
| **Vendor Extensions**  | Hardware / platform | `EMGPU_VK_push_constants`  |

---

## Design Goals

**Cross-platform** — Develop once, deploy everywhere.

**High performance** — Thin abstractions over platform-native functionality. No unnecessary overhead.

**Extensible** — New capabilities are added through Extensions, never by breaking the Core protocol.

**Multi-implementation** — Applications target the Protocol, not a specific Driver.

**Future-proof** — The protocol can evolve ahead of any implementation, giving the ecosystem room to grow.

### What makes Ember different from other libraries

Most cross-platform libraries are both the **API** and the **implementation**. Applications depend directly on that implementation, meaning the library itself becomes the long-term compatibility layer.

As new technologies emerge, **Drivers** can expose them through **Extensions** without requiring applications to rewrite their foundation.
Over time, widely adopted Extensions can become part of newer protocol revisions while existing applications and older Drivers continue to exist.

---

## Philosophy

Ember will never implement the following:
* Common data structures in the core protocol.
* New technology immediatly after release.
* Any Ember-specific concepts, it will always be aplicable to the rest of the industry.
* Support for any platform no matter how specific.
* Any metaprogramming concepts.
* Any external dependencies except the OS itself.

Many frameworks attempt to hide the operating system. Ember takes a different approach.

Ember exposes modern platform capabilities through a consistent protocol while preserving access to advanced functionality through Extensions. The goal is not the lowest common denominator — it's a stable foundation that grows with hardware, operating systems, and software ecosystems.

**Ember is a Protocol. Build on the contract, not the implementation.**
