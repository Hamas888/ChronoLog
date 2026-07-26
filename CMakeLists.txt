cmake_minimum_required(VERSION 3.15)
project(ChronoLog VERSION 1.2.0 LANGUAGES CXX)

set(CHRONOLOG_VERSION 1.2.0)

set(CHRONOLOG_IS_STM32 FALSE)
if(CMAKE_SYSTEM_NAME STREQUAL "Generic" OR
   CMAKE_C_COMPILER_TARGET MATCHES "arm-none-eabi" OR
   CMAKE_CXX_COMPILER_TARGET MATCHES "arm-none-eabi" OR
   CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|ARM|armv)")
  set(CHRONOLOG_IS_STM32 TRUE)
endif()

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

# STM32 / generic embedded CMake project
elseif(CHRONOLOG_IS_STM32)
    add_library(ChronoLog STATIC
        src/ChronoLog.cpp
        src/ChronoLogRemote.cpp
    )
    target_include_directories(ChronoLog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_features(ChronoLog PUBLIC cxx_std_17)
    target_compile_definitions(ChronoLog PUBLIC
        USE_HAL_DRIVER=1
    )

# Desktop/native CMake project
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
if(NOT DEFINED ESP_PLATFORM AND NOT DEFINED IDF_VER AND NOT DEFINED ENV{IDF_PATH} AND NOT DEFINED ZEPHYR_BASE AND NOT CHRONOLOG_IS_STM32)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(examples)
endif()