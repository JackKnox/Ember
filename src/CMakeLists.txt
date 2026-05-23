file(GLOB_RECURSE EMBER_SOURCES "./core/**.c")
add_library(ember STATIC ${EMBER_SOURCES})
target_include_directories(ember PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")

target_compile_definitions(ember PUBLIC
    $<$<CONFIG:Debug>:EMBER_BUILD_DEBUG>
    $<$<CONFIG:Release>:EMBER_BUILD_RELEASE>
    $<$<BOOL:${EMBER_DEV}>:EMBER_DEV>
    $<$<BOOL:${EMBER_DIST}>:EMBER_DIST>)

# Ember platform abstraction module
# --------------------------------
file(GLOB_RECURSE EMPLAT_SOURCES "./platform/**.c")
add_library(ember_platform STATIC ${EMPLAT_SOURCES})

target_link_libraries(ember_platform PUBLIC ember)
target_include_directories(ember_platform PRIVATE "./platform/")
# --------------------------------

# Ember window abstraction module
# --------------------------------
file(GLOB_RECURSE EMWIN_SOURCES "./window/**.c")
add_library(ember_window STATIC ${EMWIN_SOURCES})

target_link_libraries(ember_window PUBLIC ember ember_platform glfw)
target_include_directories(ember_window PRIVATE "./window/")
# --------------------------------

# Ember gpu abstraction module
# --------------------------------
file(GLOB_RECURSE EMGPU_SOURCES "./gpu/**.c")
add_library(ember_gpu STATIC ${EMGPU_SOURCES})

find_package(Vulkan REQUIRED)
target_link_libraries(ember_gpu PUBLIC ember ember_platform Vulkan::Vulkan)
target_include_directories(ember_gpu PRIVATE "./gpu/")
# --------------------------------