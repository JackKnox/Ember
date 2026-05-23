add_executable(emwin_window "./emwin_window.c")
target_link_libraries(emwin_window PRIVATE ember ember_window)

add_executable(emgpu_surface "./emgpu_surface.c")
target_link_libraries(emgpu_surface PRIVATE ember ember_window ember_gpu)