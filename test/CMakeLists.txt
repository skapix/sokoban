
find_package(GTest)

if (NOT GTEST_FOUND)
  message(STATUS "GTest library is not found. Tests will not be built.")
  return()
endif()

enable_testing()

add_executable(soko_tests soko/test_util.cpp soko/test_hungarian_algo.cpp
  soko/test_heuristic.cpp soko/test_solver.cpp)
target_link_libraries(soko_tests GTest::GTest GTest::Main sokolib)


add_test(NAME tests COMMAND soko_tests)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS soko_tests)

