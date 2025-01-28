cmake_minimum_required(VERSION 3.16)

project("onitama")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

set(Src
        src/main.cpp
)

set(Header)

add_executable("onitama" ${Src} ${Header})

if (MSVC)
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
else ()
    set(InstallDest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()
