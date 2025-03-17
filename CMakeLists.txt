cmake_minimum_required(VERSION 3.29)

project("onitama")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

set(Src
        src/main.cpp
        src/gameMaster.cpp

        src/game/game.cpp
        src/game/board.cpp
        src/game/card.cpp
        src/game/move.cpp

        src/util/ansiColor.cpp
        src/util/color.cpp
        src/util/coordinate.cpp
        src/util/offset.cpp
        src/util/winState.cpp
        src/util/stateGraph.cpp
        src/util/parse.cpp

        src/strategies/human.cpp
        src/strategies/positional.cpp
        src/strategies/random.cpp
        src/strategies/monteCarlo.cpp
        src/strategies/minMax.cpp

        src/cli/cli.cpp
        src/cli/command.cpp
        src/cli/cards.cpp
        src/cli/game.cpp
        src/cli/strategies.cpp
        src/cli/experiment.cpp
        src/cli/print.cpp

        src/experiments/fairCards.cpp
        src/experiments/stateGraph.cpp
)

set(Header
        src/gameMaster.h
        src/constants.h

        src/game/game.h
        src/game/board.h
        src/game/piece.h
        src/game/card.h
        src/game/move.h

        src/util/ansiColor.h
        src/util/color.h
        src/util/coordinate.h
        src/util/offset.h
        src/util/winState.h
        src/util/base64.h
        src/util/stateGraph.h
        src/util/parse.h

        src/strategies/strategy.h
        src/strategies/human.h
        src/strategies/positional.h
        src/strategies/random.h
        src/strategies/monteCarlo.h
        src/strategies/minMax.h

        src/cli/cli.h
        src/cli/command.h
        src/cli/cards.h
        src/cli/game.h
        src/cli/strategies.h
        src/cli/experiment.h
        src/cli/print.h

        src/experiments/fairCards.h
        src/experiments/stateGraph.h
)

set(INCLUDE_DIRS)


# stb dependency
if(NOT IS_READABLE "${CMAKE_SOURCE_DIR}/vendor/stb/stb_image_write.h")
    file(DOWNLOAD
        https://raw.githubusercontent.com/nothings/stb/1ee679ca2ef753a528db5ba6801e1067b40481b8/stb_image_write.h
        "${CMAKE_SOURCE_DIR}/vendor/stb/stb_image_write.h"
    )
endif()

add_definitions(-DSTB_IMAGE_WRITE_IMPLEMENTATION)

list(APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/stb")


add_executable("onitama" ${Src} ${Header})

target_include_directories(onitama PRIVATE ${INCLUDE_DIRS})

if (MSVC)
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
else ()
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()
