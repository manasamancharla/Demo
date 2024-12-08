
# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Demo VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define ASIO as an interface library
add_library(asio INTERFACE)

# Enable ASIO standalone mode
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)


# Find all source files in the networking directory and main.cpp
file(GLOB_RECURSE SOURCES
    networking/*.hpp
    networking/*.cpp
    main.cpp
)


# Include other headers
target_include_directories(asio INTERFACE ${CMAKE_SOURCE_DIR}/asio-1.30.2/include)


# # For UNIX-like systems (Linux, macOS)
# if(UNIX)
#     target_link_libraries(Demo PRIVATE pthread)
# endif()

# # For Windows
# if(WIN32)
#     add_definitions(-D_WIN32_WINNT=0x0A00)
# endif()


# Add the executable with all source files
add_executable(Demo ${SOURCES})

# Link the ASIO library with the executable
target_link_libraries(Demo PRIVATE asio)





