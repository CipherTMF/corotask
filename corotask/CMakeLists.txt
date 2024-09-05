project(corotask)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_library(corotask
        corotask
        network/server.cpp
        network/server.h
        threads/coro_pool.cpp
        threads/coro_pool.h
        tasks/task.hpp
        tasks/awaitables.hpp
        interface/root.hpp
        model/rank.cpp
        model/rank.h
        network/client.cpp
        network/client.h
        common/logging.h
        common/logging.cpp
        common/traits.h
        network/prot.hpp
        interface/exec.hpp
        threads/network_pool.cpp
        threads/network_pool.h
        common/timeit.hpp
        common/profiling.cpp
        common/profiling.h
        common/table.cpp
        common/table.h
)

target_include_directories(corotask PRIVATE ../)
target_link_libraries(corotask PUBLIC Qt6::Core Qt6::WebSockets)