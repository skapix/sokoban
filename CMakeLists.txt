cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (
    sokoban
    VERSION 0.1.0
    DESCRIPTION "Sokoban-solver"
)

include(cmake/werror.cmake)
include(cmake/util.cmake)

add_subdirectory(src)
add_subdirectory(test)
