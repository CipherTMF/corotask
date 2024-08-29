project(corotask)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_library(corotask
        corotask
        network/server.cpp
        network/server.h
        threads/pool.cpp
        threads/pool.h
)

target_link_libraries(corotask PUBLIC Qt6::Core Qt6::WebSockets)