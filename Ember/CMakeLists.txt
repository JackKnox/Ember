file(GLOB_RECURSE EMBER_SOURCES "src/core/**.c")
add_library(ember STATIC ${EMBER_SOURCES})
target_include_directories(ember PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(ember PUBLIC
    $<$<CONFIG:Debug>:EMBER_BUILD_DEBUG>
    $<$<CONFIG:Release>:EMBER_BUILD_RELEASE>
    $<$<BOOL:${EMBER_DEV}>:EMBER_DEV>
    $<$<BOOL:${EMBER_DIST}>:EMBER_DIST>)

# Ember platform abstraction module
# --------------------------------
file(GLOB_RECURSE EMPLAT_SOURCES "src/platform/**.c")
add_library(ember_platform STATIC ${EMPLAT_SOURCES})

target_link_libraries(ember_platform PUBLIC ember)
target_include_directories(ember_platform PRIVATE "src/platform/")
# --------------------------------

# Ember window abstraction module
# --------------------------------
file(GLOB_RECURSE EMWIN_SOURCES "src/window/**.c")
add_library(ember_window STATIC ${EMWIN_SOURCES})

target_link_libraries(ember_window PUBLIC ember ember_platform glfw)
target_include_directories(ember_window PRIVATE "src/window/")
# --------------------------------

# Ember gpu abstraction module
# --------------------------------
file(GLOB_RECURSE EMGPU_SOURCES "src/gpu/**.c")
add_library(ember_gpu STATIC ${EMGPU_SOURCES})

find_package(Vulkan REQUIRED)
target_link_libraries(ember_gpu PUBLIC ember ember_platform Vulkan::Vulkan)
target_include_directories(ember_gpu PRIVATE "src/gpu/")
# --------------------------------

add_library(ember_gpu_wayland_surface STATIC "src/gpu/ext/wayland_surface.c"
                                             "src/gpu/ext/vulkan_wayland_surface.c")
target_link_libraries(ember_gpu_wayland_surface PUBLIC ember_gpu)
target_include_directories(ember_gpu_wayland_surface PRIVATE "src/gpu/")