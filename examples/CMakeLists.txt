add_executable(basic_window "./basic_window.c")
target_link_libraries(basic_window PRIVATE ember ember_window)

add_executable(clear_colour "./clear_colour.c")
target_link_libraries(clear_colour PRIVATE ember ember_window ember_gpu)

add_executable(rainbow_triangle "./rainbow_triangle.c")
target_link_libraries(rainbow_triangle PRIVATE ember ember_window ember_gpu)