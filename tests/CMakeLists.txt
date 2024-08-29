project(tests)

find_package(doctest REQUIRED)

add_executable(tests
        main.cpp
)

target_link_libraries(tests PUBLIC doctest::doctest)