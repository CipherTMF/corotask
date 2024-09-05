
project(segfault_handler)
add_executable(segfault_handler segfault_handler/main.cpp)

project(launch_test)
add_executable(launch_test launch_test/main.cpp)
target_include_directories(launch_test PUBLIC ../)
target_link_libraries(launch_test PRIVATE corotask)