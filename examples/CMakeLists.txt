cmake_minimum_required(VERSION 3.15)
project(ChronoLogExamples NONE)

# Build selected desktop examples for native testing.
function(add_example target source match_output)
  add_executable(${target} ${source})
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  target_compile_features(${target} PRIVATE cxx_std_17)
  if(TARGET ChronoLog)
    target_link_libraries(${target} PRIVATE ChronoLog)
  endif()

  add_test(NAME ${target}Example COMMAND ${target})
  set_tests_properties(${target}Example PROPERTIES
    TIMEOUT 15
    PASS_REGULAR_EXPRESSION "${match_output}"
  )
endfunction()

add_example(DesktopLogging Desktop/Logging/Logging.cpp "Desktop Logging Example Starting")
add_example(DesktopProgressBar Desktop/ProgressBar/ProgressBar.cpp "Simulating Download")
add_example(DesktopThreadSafety Desktop/ThreadSafety/ThreadSafety.cpp "Desktop Thread Safety Example Starting")
add_example(DesktopRemoteLogging Desktop/RemoteLogging/RemoteLogging.cpp "Desktop Remote Logging Example Starting")
