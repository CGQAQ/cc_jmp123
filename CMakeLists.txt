cmake_minimum_required(VERSION 3.17)
project(jmp123)

set(CMAKE_CXX_STANDARD 20)

add_executable(jmp123 main.cpp)

target_sources(jmp123 PUBLIC decoder/bit_stream.cpp decoder/header.cpp)

add_subdirectory(third_party/fmt)

# fmt library
target_include_directories(jmp123 PUBLIC third_party/fmt/include)
target_link_libraries(jmp123 fmt::fmt)