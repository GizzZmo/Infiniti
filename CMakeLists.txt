cmake_minimum_required(VERSION 3.16)
project(Infiniti VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/O2 /W3)
else()
    add_compile_options(-O2 -Wall -Wextra)
endif()

set(ENGINE_SOURCES
    src/main.cpp
    src/bitboard.cpp
    src/position.cpp
    src/movegen.cpp
    src/eval.cpp
    src/search.cpp
    src/uci.cpp
    nnue/nnue.cpp
    nnue/nnue_file.cpp
    nnue/features_halfkp.cpp
    nnue/accumulator.cpp
    nnue/network.cpp
)

add_executable(infiniti ${ENGINE_SOURCES})
target_include_directories(infiniti PRIVATE ${CMAKE_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(infiniti PRIVATE Threads::Threads)

enable_testing()
add_test(NAME uci_sanity
    COMMAND ${CMAKE_COMMAND} -E echo "Running UCI sanity..."
)
