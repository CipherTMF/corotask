cmake_minimum_required(VERSION 3.28)
project(corotask)

# Mandatory compiler settings
set(CMAKE_CXX_STANDARD 20)

# Sub projects
add_subdirectory(corotask)
add_subdirectory(ct-agent)
add_subdirectory(tests)
