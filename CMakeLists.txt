cmake_minimum_required(VERSION 3.17)
project(jmp123)

set(CMAKE_CXX_STANDARD 20)

add_executable(jmp123 main.cpp)

target_sources(jmp123 PUBLIC decoder/bit_stream.cpp)