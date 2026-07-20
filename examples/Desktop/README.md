# ChronoLog Desktop Examples

This directory contains several examples demonstrating how to use the ChronoLog library in a standard C++ desktop environment (Windows, Linux, macOS).

## Building the Examples

All desktop examples are built using CMake. This provides a cross-platform, modern, and easy-to-use build system.

### Prerequisites

- A C++ compiler (e.g., GCC, Clang, MSVC)
- CMake (version 3.11 or newer)
- Git

### Build Steps

1.  **Create and navigate to a `build` directory:**
    Open a terminal inside the `Desktop` examples folder. Create a new directory for the build output (a common name is `build`) and navigate into it.

    ```bash
    mkdir build
    cd build
    ```

2.  **Configure the project with CMake:**
    Run CMake to generate the build files for your system. This step also automatically fetches the ChronoLog library from its Git repository.

    ```bash
    cmake ..
    ```

3.  **Compile the examples:**
    Run the build command to compile all the examples.

    ```bash
    cmake --build .
    ```

    After the build completes, you will find the executables for each example inside the `build` directory.

## Examples

### 1. Logging

-   **Executable:** `Logging`
-   **Source:** `Logging/Logging.cpp`
-   **Description:** A basic demonstration of the core logging functionality. It shows how to initialize a logger and print messages at different severity levels (INFO, WARN, ERROR, etc.).

### 2. ProgressBar

-   **Executable:** `ProgressBar`
-   **Source:** `ProgressBar/ProgressBar.cpp`
-   **Description:** Demonstrates the progress bar feature. This is useful for visualizing the progress of long-running tasks, such as file downloads, data processing, or firmware updates.

### 3. RemoteLogging

-   **Executable:** `RemoteLogging`
-   **Source:** `RemoteLogging/RemoteLogging.cpp`
-   **Description:** Shows how to configure ChronoLog to send log messages to a remote server. This example is a template for integrating with remote logging services or custom backend solutions.

### 4. ThreadSafety

-   **Executable:** `ThreadSafety`
-   **Source:** `ThreadSafety/ThreadSafety.cpp`
-   **Description:** This example demonstrates the thread-safe capabilities of ChronoLog. It spawns multiple threads that log concurrently to the same logger instance, showcasing the built-in mutex protection that prevents garbled output.
