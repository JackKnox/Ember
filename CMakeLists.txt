cmake_minimum_required(VERSION 3.16)
project(Ember VERSION 0.5.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

option(EMBER_BUILD_EXAMPLES "Enable building examples and linking nessacery dependices" ${MAIN_PROJECT})

add_subdirectory(shared)

if (EMBER_BUILD_EXAMPLES)
    message(STATUS "Building EMBER examples with ember-native driver.")
    add_subdirectory(examples)
endif()