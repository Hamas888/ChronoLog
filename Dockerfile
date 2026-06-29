# =============================================================================
# ChronoLog CI Image
# =============================================================================
# Multi-platform CI image for building & testing ChronoLog on:
#   - Desktop (Linux, g++, CMake, CTest)
#   - ESP32 (Arduino framework, ESP-IDF framework)
#   - STM32
#   - nRF52
#   - AVR (Arduino Uno, etc.)
#
# Uses PlatformIO CLI to cross-compile for all embedded targets.
# =============================================================================

FROM ubuntu:24.04

LABEL org.opencontainers.image.source="https://github.com/Hamas888/ChronoLog"
LABEL org.opencontainers.image.description="ChronoLog CI image — PlatformIO + CMake for embedded & desktop testing"
LABEL org.opencontainers.image.licenses="MIT"

# ---------------------------------------------------------------------------
# Prevent tzdata / apt from hanging on interactive prompts
# ---------------------------------------------------------------------------
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# ---------------------------------------------------------------------------
# System dependencies
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build tools for desktop tests
    build-essential \
    cmake \
    g++ \
    make \
    # PlatformIO / Python
    python3 \
    python3-pip \
    python3-venv \
    python3-dev \
    # Utilities
    git \
    wget \
    curl \
    ca-certificates \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# Install PlatformIO CLI
# ---------------------------------------------------------------------------
RUN pip3 install --break-system-packages --no-cache-dir platformio

# ---------------------------------------------------------------------------
# Pre-cache PlatformIO platforms & toolchains
# ---------------------------------------------------------------------------
# Create a dummy project to force PlatformIO to download all needed cores.
# This avoids downloading them on every CI run.
WORKDIR /tmp/pio-cache

RUN mkdir -p src && \
    echo "void setup(){} void loop(){}" > src/sketch.ino

# ESP32 — Arduino framework
RUN printf "[env:esp32-s3]\nplatform = espressif32\nboard = esp32-s3-devkitm-1\nframework = arduino\n" > platformio.ini && \
    pio pkg install --skip-dependencies 2>&1 | tail -5 && \
    rm -f platformio.ini

# ESP32 — ESP-IDF framework
RUN printf "[env:esp32-s3]\nplatform = espressif32\nboard = esp32-s3-devkitm-1\nframework = espidf\n" > platformio.ini && \
    pio pkg install --skip-dependencies 2>&1 | tail -5 && \
    rm -f platformio.ini

# STM32
RUN printf "[env:stm32]\nplatform = ststm32\nboard = genericSTM32F103CB\nframework = arduino\n" > platformio.ini && \
    pio pkg install --skip-dependencies 2>&1 | tail -5 && \
    rm -f platformio.ini

# nRF52
RUN printf "[env:nrf52]\nplatform = nordicnrf52\nboard = nrf52840dk\nframework = arduino\n" > platformio.ini && \
    pio pkg install --skip-dependencies 2>&1 | tail -5 && \
    rm -f platformio.ini

# Arduino AVR
RUN printf "[env:avr]\nplatform = atmelavr\nboard = uno\nframework = arduino\n" > platformio.ini && \
    pio pkg install --skip-dependencies 2>&1 | tail -5 && \
    rm -f platformio.ini

# Clean up the cache project
WORKDIR /
RUN rm -rf /tmp/pio-cache

# ---------------------------------------------------------------------------
# Verify installations
# ---------------------------------------------------------------------------
RUN cmake --version | head -1 && \
    g++ --version | head -1 && \
    pio --version

# ---------------------------------------------------------------------------
# Default: print tool versions
# ---------------------------------------------------------------------------
CMD ["pio", "--version"]
