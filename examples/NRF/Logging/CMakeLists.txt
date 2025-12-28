cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Logging)

target_sources(app PRIVATE src/Logging.cpp)

# Include ChronoLog headers
target_include_directories(app PRIVATE ../../../include)
