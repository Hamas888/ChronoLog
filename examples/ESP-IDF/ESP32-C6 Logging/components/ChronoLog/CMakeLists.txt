# ChronoLog/CMakeLists.txt

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
else()
    message(FATAL_ERROR "ChronoLog: Unknown build system. Use Zephyr or ESP-IDF.")
endif()