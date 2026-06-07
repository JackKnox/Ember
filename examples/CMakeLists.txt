add_executable(hello_window "./hello_window.c")
target_link_libraries(hello_window PRIVATE ember-shared)

add_executable(clear_screen "./clear_screen.c")
target_link_libraries(clear_screen PRIVATE ember-shared)
