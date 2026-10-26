cmake_minimum_required(VERSION 4.3)
project(Chess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

set(RAYLIB_VERSION 6.2)
FetchContent_Declare(raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)

FetchContent_MakeAvailable(raylib)

set(SOURCES
    src/main.cpp
    src/game/game.cpp
    src/chess/fen.cpp
    src/chess/perft.cpp
    src/chess/chess.cpp
    src/chess/lookup.cpp
    src/chess/movegen.cpp
    src/engine/engine.cpp
    src/graphics/assets.cpp
    src/graphics/renderer.cpp
    
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wpedantic)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib stdc++exp)