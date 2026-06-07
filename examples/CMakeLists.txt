add_executable(hello_window "./hello_window.c")
target_link_libraries(hello_window PRIVATE ember-shared)

add_executable(clear_screen "./clear_screen.c")
target_link_libraries(clear_screen PRIVATE ember-shared)

add_executable(file_io "./file_io.c")
target_link_libraries(file_io PRIVATE ember-shared)

add_executable(timers "./timers.c")
target_link_libraries(timers PRIVATE ember-shared)