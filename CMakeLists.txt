cmake_minimum_required(VERSION 3.16)
project(Ember VERSION 0.2 LANGUAGES C)

include(FetchContent)

set(CMAKE_C_STANDARD 99)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

set(DEV_MODE OFF)
if (${MAIN_PROJECT})
    message(STATUS "Enabling DEV mode as Ember is the main project")
    set(DEV_MODE ON)
endif()

add_subdirectory(Ember)
add_subdirectory(sandbox)

FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(GLFW)