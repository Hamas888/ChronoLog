# ChronoLog/CMakeLists.txt

set(CHRONOLOG_VERSION 1.0.0)

# Check if we're building with Zephyr
if(DEFINED ZEPHYR_BASE)

    # zephyr_library()
    # zephyr_library_sources() → not needed yet
    zephyr_include_directories(include)

# Check if we're building with ESP-IDF
elseif(IDF_PROJECT)
    idf_component_register(
        # SRCS "src/ChronoLog.cpp" → not needed yet
        INCLUDE_DIRS "include"
    )
    
# STM32 / generic CMake project
else()
    add_library(ChronoLog INTERFACE)
    target_include_directories(ChronoLog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(ChronoLog INTERFACE cxx_std_17)
endif()