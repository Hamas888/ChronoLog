# ChronoLog/CMakeLists.txt

set(CHRONOLOG_VERSION 1.1.5)

# Check if we're building with Zephyr
if(DEFINED ZEPHYR_BASE)

    # zephyr_library()
    # zephyr_library_sources() → not needed yet
    zephyr_include_directories(src)

# Check if we're building with ESP-IDF
elseif(DEFINED ESP_PLATFORM OR DEFINED IDF_VER OR DEFINED ENV{IDF_PATH})
    idf_component_register(
        # SRCS "src/ChronoLog.cpp" → not needed yet
        INCLUDE_DIRS "src"
        REQUIRES
            "lwip"
            "esp_wifi"
            "esp_event"
            "freertos"
    )
    
# STM32 / generic CMake project
else()
    add_library(ChronoLog INTERFACE)
    target_include_directories(ChronoLog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_features(ChronoLog INTERFACE cxx_std_17)
endif()