cmake_minimum_required(VERSION 3.10)

project(Proxy-Server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(my_interface_library INTERFACE)

target_compile_definitions(my_interface_library INTERFACE ASIO_STANDALONE)

file(GLOB_RECURSE SOURCES
    proxy-1.0.0/*.hpp
    proxy-1.0.0/*.cpp
    main.cpp
)

target_include_directories(my_interface_library INTERFACE ${CMAKE_SOURCE_DIR}/asio-1.30.2/include)
target_include_directories(my_interface_library INTERFACE ${CMAKE_SOURCE_DIR}/proxy-1.0.0/)

add_executable(Proxy ${SOURCES})

target_link_libraries(Proxy PRIVATE my_interface_library)




