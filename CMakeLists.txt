cmake_minimum_required(VERSION 3.16)

project("onitama")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

set(Src
        src/main.cpp
        src/gameMaster.cpp

        src/game/game.cpp
        src/game/board.cpp
        src/game/piece.cpp
        src/game/card.cpp
        src/game/move.cpp

        src/util/color.cpp
        src/util/coordinate.cpp
        src/util/offset.cpp

        src/strategies/human.cpp
)

set(Header
        src/gameMaster.h

        src/game/game.h
        src/game/board.h
        src/game/piece.h
        src/game/card.h
        src/game/move.h

        src/util/color.h
        src/util/coordinate.h
        src/util/offset.h

        src/strategies/strategy.h
        src/strategies/human.h
)

add_executable("onitama" ${Src} ${Header})

if (MSVC)
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
else ()
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()
