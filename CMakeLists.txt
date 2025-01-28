cmake_minimum_required(VERSION 3.16)

project("onitama")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

set(Src
        src/main.cpp

        src/game/game.cpp
        src/game/board.cpp
        src/game/piece.cpp
        src/game/card.cpp
)

set(Header
        src/game/game.h
        src/game/board.h
        src/game/piece.h
        src/game/card.h
)

add_executable("onitama" ${Src} ${Header})

if (MSVC)
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
else ()
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()
