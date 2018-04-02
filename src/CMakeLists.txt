set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------
# soko library
#
set(sokolib_cpp soko/map.cpp soko/game_state.cpp)
set(sokolib_h soko/map.h soko/cell.h soko/mat.hpp soko/game_state.h)

add_library(sokolib STATIC ${sokolib_cpp} ${sokolib_h})

#-----------------------------
# soko application
#

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED) # only headers

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(interface_cpp interface/main_window.cpp interface/scene.cpp interface/elements.cpp interface/util.cpp)
set(interface_h interface/main_window.h interface/scene.h interface/elements.h interface/util.h)

add_executable(sokoapp main.cpp ${CMAKE_SOURCE_DIR}/data.qrc ${interface_cpp} ${interface_h})
target_link_libraries(sokoapp sokolib Qt5::Widgets)

target_include_directories(sokoapp PRIVATE ${Boost_INCLUDE_DIRS})

add_custom_command(
        TARGET sokoapp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/levels.txt
                ${CMAKE_CURRENT_BINARY_DIR}/levels.txt
                DEPENDS ${CMAKE_SOURCE_DIR}/levels.txt)

