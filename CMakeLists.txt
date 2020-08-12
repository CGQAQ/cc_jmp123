cmake_minimum_required(VERSION 3.17)
project(jmp123)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif ()

add_executable(jmp123 main.cc output/output.cc output/output.h playback.cc playback.h)
target_include_directories(jmp123 PUBLIC .)

# link pThread
find_package(Threads)
target_link_libraries(jmp123 ${CMAKE_THREAD_LIBS_INIT})

target_sources(
        jmp123 PUBLIC
        decoder/audio_buffer.cc
        decoder/audio_buffer.h
        decoder/bit_stream.cc
        decoder/bit_stream.h
        decoder/header.cc
        decoder/header.h
        decoder/tables.cc
        decoder/tables.h
        decoder/layer_1.cc
        decoder/layer_1.h
        decoder/layer123.cc
        decoder/layer123.h
        decoder/synthesis.cc
        decoder/synthesis.h
        decoder/layer_2.cc decoder/layer_2.h
        decoder/layer_3.cc decoder/layer_3.h
        decoder/synthesis_concurrent.cc decoder/synthesis_concurrent.h
        decoder/bit_stream_main_data.cc decoder/bit_stream_main_data.h
)

add_subdirectory(third_party/fmt)
# fmt library
target_include_directories(jmp123 PUBLIC third_party/fmt/include)
target_link_libraries(jmp123 fmt::fmt)

set(RTAUDIO_BUILD_STATIC_LIBS)
set(RTAUDIO_API_JACK OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/rtaudio)
target_include_directories(jmp123 PUBLIC third_party/rtaudio)
target_link_libraries(jmp123 rtaudio)