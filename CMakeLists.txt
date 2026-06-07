cmake_minimum_required(VERSION 3.16)
project(Ember VERSION 0.5.0 LANGUAGES C)

include(FetchContent)

set(CMAKE_C_STANDARD 99)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

option(EMBER_DIST "Strip ember of all safety features for distribution, not recommended for development")
option(EMBER_DEV  "Build ember with developer-only tooling" ${MAIN_PROJECT})

add_subdirectory(shared)
add_subdirectory(examples)