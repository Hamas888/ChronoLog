cmake_minimum_required(VERSION 3.15)
project(ChronoLogTests LANGUAGES CXX)

add_executable(desktop_example_test desktop_example_test.cpp)
target_include_directories(desktop_example_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_compile_features(desktop_example_test PRIVATE cxx_std_17)

if(TARGET ChronoLog)
  target_link_libraries(desktop_example_test PRIVATE ChronoLog)
endif()

add_test(NAME DesktopExampleTest COMMAND desktop_example_test)
set_tests_properties(DesktopExampleTest PROPERTIES LABELS "desktop")
