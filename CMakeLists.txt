cmake_minimum_required(VERSION 3.15)
project(ChronoLog VERSION 1.2.0 LANGUAGES CXX)

set(CHRONOLOG_VERSION 1.2.0)

# Check if we're building with Zephyr
if(DEFINED ZEPHYR_BASE)

    zephyr_library()
    zephyr_library_sources(src/ChronoLog.cpp src/ChronoLogRemote.cpp)
    zephyr_include_directories(src)

# Check if we're building with ESP-IDF
elseif(DEFINED ESP_PLATFORM OR DEFINED IDF_VER OR DEFINED ENV{IDF_PATH})
    idf_component_register(
        INCLUDE_DIRS "src"
        SRCS "src/ChronoLog.cpp" "src/ChronoLogRemote.cpp"
        REQUIRES
            "lwip"
            "esp_wifi"
            "esp_event"
            "freertos"
    )
    
# STM32 / generic CMake project
else()
    add_library(ChronoLog STATIC
        src/ChronoLog.cpp
        src/ChronoLogRemote.cpp
    )
    target_include_directories(ChronoLog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_features(ChronoLog PUBLIC cxx_std_17)
    target_compile_definitions(ChronoLog PUBLIC
        CHRONOLOG_PRO_FEATURES=1
        CHRONOLOG_REMOTE_ENABLE=1
        CHRONOLOG_COLOR_ENABLE=1
    )
endif()

# Desktop/native CMake tests and desktop examples
if(NOT DEFINED ESP_PLATFORM AND NOT DEFINED IDF_VER AND NOT DEFINED ENV{IDF_PATH} AND NOT DEFINED ZEPHYR_BASE)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(examples)
endif()