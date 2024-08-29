project(ct-agent)

add_executable(ct-agent
    main.cpp
)

target_link_libraries(ct-agent PRIVATE corotask)