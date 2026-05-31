file(GLOB_RECURSE EMBER_SOURCES "./core/**.c")
add_library(ember STATIC ${EMBER_SOURCES})
target_include_directories(ember PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")

target_compile_definitions(ember PUBLIC
    EMBER_BUILD_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
    EMBER_BUILD_RELEASE=$<IF:$<CONFIG:Release>,1,0>

    EMBER_DEV=$<IF:$<BOOL:${EMBER_DEV}>,1,0>
    EMBER_DIST=$<IF:$<BOOL:${EMBER_DIST}>,1,0>)

# Ember platform abstraction module
# --------------------------------
file(GLOB_RECURSE EMPLAT_SOURCES "./platform/platform_posix.c")
add_library(ember_platform STATIC ${EMPLAT_SOURCES})

target_link_libraries(ember_platform PUBLIC ember)
target_include_directories(ember_platform PRIVATE "./platform/")
# --------------------------------

# Ember window abstraction module
# --------------------------------
file(GLOB_RECURSE EMWIN_SOURCES "./window/global.c" 
                                "./window/wayland_win.c"
                                "./window/wayland_embergpu.c")
add_library(ember_window STATIC ${EMWIN_SOURCES})

target_link_libraries(ember_window PUBLIC ember ember_platform wayland-client)
target_include_directories(ember_window PRIVATE "./window/")
# --------------------------------

# Ember gpu abstraction module
# --------------------------------
file(GLOB_RECURSE EMGPU_SOURCES "./gpu/device.c" 
                                "./gpu/extension.c" 
                                "./gpu/frame.c" 
                                "./gpu/resources.c"
                                "./gpu/vulkan/**.c")
add_library(ember_gpu STATIC ${EMGPU_SOURCES})

find_package(Vulkan REQUIRED)
target_link_libraries(ember_gpu PUBLIC ember ember_platform Vulkan::Vulkan)
target_include_directories(ember_gpu PRIVATE "./gpu/")
# --------------------------------