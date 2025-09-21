# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_language(CXX)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nRF52832_Logging)

target_sources(app PRIVATE src/main.cpp)
add_subdirectory(modules/ChronoLog)
