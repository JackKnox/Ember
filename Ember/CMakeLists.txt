file(GLOB_RECURSE EMBER_SOURCES "src/core/**.c")
add_library(ember STATIC ${EMBER_SOURCES})

target_compile_definitions(ember PUBLIC
    $<$<CONFIG:Debug>:EMBER_BUILD_DEV>
    $<$<CONFIG:Release>:EMBER_BUILD_RELEASE>
    $<$<CONFIG:Dist>:EMBER_BUILD_DIST>
)

target_include_directories(ember PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Ember platform abstraction module
# --------------------------------
file(GLOB_RECURSE EMPLAT_SOURCES "src/platform/**.c")
add_library(ember_platform STATIC ${EMPLAT_SOURCES})

target_link_libraries(ember_platform PUBLIC ember glfw)
target_include_directories(ember_platform PRIVATE "src/platform/")
# --------------------------------

# Ember gpu abstraction module
# --------------------------------
file(GLOB_RECURSE EMGPU_SOURCES "src/gpu/**.c")
add_library(ember_gpu STATIC ${EMGPU_SOURCES})

find_package(Vulkan REQUIRED)

target_link_libraries(ember_gpu PUBLIC ember ember_platform Vulkan::Vulkan)
target_include_directories(ember_gpu PRIVATE "src/gpu/")
# --------------------------------