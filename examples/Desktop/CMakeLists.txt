# CMakeLists.txt for ChronoLog Desktop Examples
# This file builds all desktop examples and serves as a reference for integrating ChronoLog.

cmake_minimum_required(VERSION 3.11)

# Project name and language
project(ChronoLogDesktopExamples CXX)

# Set C++ standard for the examples
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch ChronoLog from Git ---
# Use FetchContent to automatically download the ChronoLog library.
# This makes the examples self-contained and easy to build.
include(FetchContent)

FetchContent_Declare(
  chronolog
  GIT_REPOSITORY "https://github.com/Hamas888/ChronoLog.git"
  GIT_TAG        "v1.3.0_Logging_Interfaces" # Or a specific version tag, e.g., "v1.0.0"
)

FetchContent_MakeAvailable(chronolog)

# --- ChronoLog Library Configuration Options ---
# These options allow you to configure ChronoLog features at build time.
# They correspond to the CHRONOLOG_* macros in the source code.

option(CHRONOLOG_MODE "Enable all logging functionality" ON)
option(CHRONOLOG_THREAD_SAFE "Enable thread-safe logging (requires pthreads)" ON)
option(CHRONOLOG_COLOR_ENABLE "Enable ANSI color-coded output" ON)
option(CHRONOLOG_PRO_FEATURES "Enable Pro features (e.g., progress bars)" ON)
option(CHRONOLOG_REMOTE_ENABLE "Enable remote TCP logging" ON)

# --- Define the ChronoLog Library ---
# We create a static library for ChronoLog that our examples can link against.
# This is the standard way to handle dependencies in CMake.

# Add the ChronoLog source files. The path is relative to this CMakeLists.txt.
add_library(chronolog_lib STATIC
    ${chronolog_SOURCE_DIR}/src/ChronoLog.cpp
    ${chronolog_SOURCE_DIR}/src/ChronoLogRemote.cpp
)

# Add the 'src' directory to the library's include path so it can find "ChronoLog.h".
target_include_directories(chronolog_lib PUBLIC ${chronolog_SOURCE_DIR}/src)

# --- Pass Configuration to the Compiler ---
# Add compile definitions based on the CMake options selected above.
if(CHRONOLOG_MODE)
    target_compile_definitions(chronolog_lib PUBLIC -DCHRONOLOG_MODE=1)
else()
    target_compile_definitions(chronolog_lib PUBLIC -DCHRONOLOG_MODE=0)
endif()

if(CHRONOLOG_THREAD_SAFE)
    target_compile_definitions(chronolog_lib PUBLIC -DCHRONOLOG_THREAD_SAFE=1)
endif()

if(CHRONOLOG_COLOR_ENABLE)
    target_compile_definitions(chronolog_lib PUBLIC -DCHRONOLOG_COLOR_ENABLE=1)
endif()

if(CHRONOLOG_PRO_FEATURES)
    target_compile_definitions(chronolog_lib PUBLIC -DCHRONOLOG_PRO_FEATURES=1)
endif()

if(CHRONOLOG_REMOTE_ENABLE)
    target_compile_definitions(chronolog_lib PUBLIC -DCHRONOLOG_REMOTE_ENABLE=1)
endif()

# --- Build the Desktop Examples ---
# Each example is defined as a separate executable.

add_executable(LoggingExample Logging/Logging.cpp)
add_executable(ProgressBarExample ProgressBar/ProgressBar.cpp)
add_executable(GraphPlotterExample GraphPlotter/GraphPlotter.cpp)
add_executable(RemoteLoggingExample RemoteLogging/RemoteLogging.cpp)
add_executable(ThreadSafetyExample ThreadSafety/ThreadSafety.cpp)

# --- Link Examples Against the ChronoLog Library ---
# This tells CMake that each example needs the chronolog_lib to compile and run.
# CMake automatically handles include paths and linking.

target_link_libraries(LoggingExample PRIVATE chronolog_lib)
target_link_libraries(ProgressBarExample PRIVATE chronolog_lib)
target_link_libraries(GraphPlotterExample PRIVATE chronolog_lib)
target_link_libraries(RemoteLoggingExample PRIVATE chronolog_lib)
target_link_libraries(ThreadSafetyExample PRIVATE chronolog_lib)

# --- Handle Platform-Specific Dependencies (like pthreads) ---
# For thread safety on Linux/macOS, we need to link against the pthreads library.
if(CHRONOLOG_THREAD_SAFE AND (CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin"))
    find_package(Threads REQUIRED)
    target_link_libraries(ThreadSafetyExample PRIVATE Threads::Threads)
    # Also link other examples if they were to use threads
    target_link_libraries(LoggingExample PRIVATE Threads::Threads)
    target_link_libraries(ProgressBarExample PRIVATE Threads::Threads)
    target_link_libraries(GraphPlotterExample PRIVATE Threads::Threads)
    target_link_libraries(RemoteLoggingExample PRIVATE Threads::Threads)
endif()

# --- Installation and Output ---
# Print a message to guide the user on how to build and run.
message(STATUS "ChronoLog examples configured. To build, run 'cmake --build .'")
message(STATUS "Executables will be created in the current build directory.")
message(STATUS "Example: ./LoggingExample")
