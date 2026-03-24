cmake_minimum_required(VERSION 3.16)
project(Ember VERSION 1.0 LANGUAGES C)

include(FetchContent)

set(CMAKE_C_STANDARD 99)

# Allow custom build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

# Single-config generators (Ninja, Makefiles)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(Ember)
add_subdirectory(sandbox)

FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(GLFW)