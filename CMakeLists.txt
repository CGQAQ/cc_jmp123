cmake_minimum_required(VERSION 3.10)
project(jmp123)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3 /permissive-")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=undefined")
endif ()

set(COMMON_SOURCE
        output/output.cc output/output.h playback.cc playback.h
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
        decoder/bit_stream_main_data.cc decoder/bit_stream_main_data.h)


add_executable(jmp123 main.cc ${COMMON_SOURCE})
target_include_directories(jmp123 PUBLIC .)
target_link_libraries(jmp123 libjmp123)

add_library(libjmp123 ${COMMON_SOURCE})
target_include_directories(libjmp123 PUBLIC .)

# link pThread
find_package(Threads)
target_link_libraries(libjmp123 ${CMAKE_THREAD_LIBS_INIT})

# fmt library
add_subdirectory(third_party/fmt)
target_include_directories(libjmp123 PUBLIC third_party/fmt/include)
target_link_libraries(libjmp123 fmt)

# portaudio library
set(PA_USE_ALSA ON)
set(PA_USE_JACK OFF)
add_subdirectory(third_party/port_audio)
target_include_directories(libjmp123 PUBLIC third_party/port_audio/bindings/cpp/include)
target_link_libraries(libjmp123 portaudio)
