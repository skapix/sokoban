set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

#-----------------------------
# soko library
#
set(sokolib_cpp map.cpp game_state.cpp solver.cpp heuristic.cpp util.cpp hungarian_algo.cpp solvability.cpp)
PREPEND(sokolib_cpp "soko/" ${sokolib_cpp})
set(sokolib_h map.h cell.h mat.hpp game_state.h solver.h cross.h
  move.h heuristic.h util.h pos.h hungarian_algo.h solvability.h)
PREPEND(sokolib_h "soko/" ${sokolib_h})
add_library(sokolib STATIC ${sokolib_cpp} ${sokolib_h})
target_include_directories(sokolib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------
# soko application
#

find_package(Threads)
if (NOT Threads_FOUND)
  message(STATUS "Threads not found. Stop building soko application.")
  return()
endif()

find_package(Qt5 COMPONENTS Widgets)
if (NOT Qt5Widgets_FOUND)
  message(STATUS "Qt5 widgets not found. Stop building soko application.")
  return()
endif()

find_package(Boost) # only headers
if (NOT Boost_FOUND)
  message(STATUS "Boost not found. Stop building soko application.")
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(interface_cpp main_window.cpp scene.cpp elements.cpp util.cpp scene_label.cpp solver_wrapper.cpp)
PREPEND(interface_cpp "interface/" ${interface_cpp})
set(interface_h main_window.h scene.h elements.h util.h scene_label.h solver_wrapper.h)
PREPEND(interface_h "interface/" ${interface_h})

if (WIN32)
  set (APP_TYPE WIN32)
endif()

add_executable(sokoapp ${APP_TYPE} main.cpp ${CMAKE_SOURCE_DIR}/data.qrc ${interface_cpp} ${interface_h})
target_link_libraries(sokoapp sokolib Qt5::Widgets Threads::Threads)

target_include_directories(sokoapp PRIVATE ${Boost_INCLUDE_DIRS})

add_custom_command(
        TARGET sokoapp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/levels.txt
                ${CMAKE_CURRENT_BINARY_DIR}/levels.txt
                DEPENDS ${CMAKE_SOURCE_DIR}/levels.txt)

