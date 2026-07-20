# ChronoLog — STM32 Examples

These examples target the **Nucleo-L010RB** board using STM32CubeMX + VS Code (CMake-based build).

---

## Setup

### 1. Clone ChronoLog into your project

```bash
cd your-stm32-project
git clone https://github.com/Hamas888/ChronoLog.git
```

### 2. Add to CMakeLists.txt

```cmake
add_subdirectory(ChronoLog)
target_link_libraries(${CMAKE_PROJECT_NAME} ChronoLog)
```

### 3. Set your UART handler

ChronoLog outputs via UART on STM32. Call `setUartHandler()` before logging:

```cpp
#include "ChronoLog.h"

ChronoLogger logger("System", CHRONOLOG_LEVEL_DEBUG);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();

    logger.setUartHandler(&huart2);    // required on STM32

    logger.info("System initialized");
}
```

### 4. Build & Flash

Open the project in VS Code (STM32 extension) or build with CMake:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

---

## STM32CubeMX Integration

Each example includes a `.ioc` file.  
Open it in STM32CubeMX to configure pins, clock, and peripherals, then regenerate code.

The `.ioc` configures:
- **USART2** (115200 baud) — log output
- **SysTick** — timebase for `HAL_GetTick()`
- **FreeRTOS** (ThreadSafety example only) — CMSIS-OS v2

---

## Example Overview

| Example | Board | Key Feature |
|---|---|---|
| [Logging](STM_VSCode_Extention/Logging/NucleoL010RB/) | Nucleo-L010RB | Basic log levels with UART output |
| [ProgressBar](STM_VSCode_Extention/ProgressBar/NucleoL010RB/) | Nucleo-L010RB | In-place progress bar via UART |
| [ThreadSafety](STM_VSCode_Extention/ThreadSafety/NucleoL010RB/) | Nucleo-L010RB | Concurrent logging from FreeRTOS tasks |
