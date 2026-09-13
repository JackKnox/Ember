include(FetchContent)
FetchContent_Declare(
    ember-native
    GIT_REPOSITORY https://github.com/JackKnox/ember-native.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(ember-native)

add_executable(hello_window "./hello_window.c")
target_link_libraries(hello_window PRIVATE ember-shared ember-native)

add_executable(clear_screen "./clear_screen.c")
target_link_libraries(clear_screen PRIVATE ember-shared ember-native)
