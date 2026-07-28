cmake_minimum_required(VERSION 3.16)
project(Ember VERSION 0.5.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

file(GLOB_RECURSE EMBER_HEADERS "./protocol/**.h")
file(GLOB_RECURSE EMBER_SHARED_SOURCES "./shared/**.c")

add_library(ember-headers INTERFACE ${EMBER_HEADERS})
target_include_directories(ember-headers INTERFACE "./protocol/")

add_library(ember-shared STATIC ${EMBER_SHARED_SOURCES})
target_link_libraries(ember-shared PUBLIC ember-headers)
target_compile_definitions(ember-shared PUBLIC EMBER_DEFINE_HELPERS)

option(EMBER_BUILD_EXAMPLES "Enable building examples and linking nessacery dependices" ${MAIN_PROJECT})

if (EMBER_BUILD_EXAMPLES)
    message(STATUS "Building EMBER examples with ember-native driver.")
    add_subdirectory(examples)
endif()
