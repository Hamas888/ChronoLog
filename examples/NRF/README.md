# ChronoLog — Zephyr Examples

These examples demonstrate ChronoLog on **Zephyr-based platforms** (nRF52, nRF53, nRF91, etc.).

---

## Setup

Add ChronoLog to your Zephyr workspace via `west.yml`:

```yaml
manifest:
  projects:
    - name: chronolog
      url: https://github.com/Hamas888/ChronoLog
      revision: v1.2.0
      path: modules/lib/chronolog
  self:
    path: app
```

Or without `west`, set `ZEPHYR_MODULES` before configuring:

```bash
cmake -B build -DZEPHYR_MODULES=/path/to/ChronoLog
```

## Build

```bash
cd examples/NRF/Logging
west build -b nrf52840dk/nrf52840 .
west flash
```

Replace `nrf52840dk/nrf52840` with your target board.

---

## Example Overview

| Example | What It Shows |
|---|---|
| [Logging](Logging/) | Basic log levels (debug, info, warn, error, fatal) |
| [ProgressBar](ProgressBar/) | In-place progress bar with `logger.progress()` |
| [ThreadSafety](ThreadSafety/) | Concurrent logging from multiple Zephyr threads |
