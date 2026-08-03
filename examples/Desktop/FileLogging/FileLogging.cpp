// FileLogging Example - demonstrates the pluggable log sink interface with the
// desktop rotating per-module file sink.
//
// Requires CHRONOLOG_SINKS_ENABLE=1 (defined by the desktop CMake build).
// Output: logs/<module>.txt in the working directory (default "logs/").
//
// Run chrono_dump.py to parse/condense the logs:
//   python3 tools/chrono_dump.py --dir logs/ --out logs_compact/

#define CHRONOLOG_PRO_FEATURES          1
#define CHRONOLOG_COLOR_ENABLE          1

#include "ChronoLog.h"
#include "ChronoLogSink.h"
#include "ChronoLogFileSink.h"

#include <thread>
#include <chrono>

int main() {
    // Register the file sink (singleton).
    ChronoLogSinkRegistry::instance().add(&ChronoLogFileSink::instance());

    // Optional configuration.
    // ChronoLogFileSink::instance().setLogsDir("logs");
    // ChronoLogFileSink::instance().setRetentionDays(7);
    // ChronoLogFileSink::instance().setMaxFileBytes(0);   // 0 = no size cap
    // ChronoLogFileSink::instance().setCondensed(false);  // full-fidelity by default

    // Two modules.
    ChronoLogger app("MyApp", CHRONOLOG_LEVEL_DEBUG);
    ChronoLogger sensor("Sensor", CHRONOLOG_LEVEL_INFO);

    app.info("File sink example starting (condensed=%d)",
             (int)ChronoLogFileSink::instance().isCondensed());

    for (int i = 0; i < 5; i++) {
        app.debug("app loop iteration %d", i);
        sensor.info("sensor reading %.2f", 20.0f + (float)i * 0.5f);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    app.warn("Wrapping up the file sink example");

    // Flush any buffered file output.
    ChronoLogSinkRegistry::instance().flushAll();

    return 0;
}
