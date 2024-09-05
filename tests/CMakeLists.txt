project(tests)

find_package(doctest REQUIRED)

add_executable(tests
        main.cpp
        tasks.cpp
)

target_include_directories(tests PUBLIC ../)
target_link_libraries(tests PRIVATE corotask doctest::doctest)