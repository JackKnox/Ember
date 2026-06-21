file(GLOB_RECURSE EMBER_SOURCES "./**.c")
add_library(ember-shared STATIC ${EMBER_SOURCES})
target_include_directories(ember-shared PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../protocol")
target_compile_definitions(ember-shared PUBLIC EMBER_DEFINE_HELPERS)
