#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include "../src/ChronoLog.h"

static bool captureStdout(const std::function<void()>& fn, std::string& output) {
    fflush(stdout);
    int stdout_fd = fileno(stdout);
#if defined(_WIN32)
    int saved_fd = _dup(stdout_fd);
#else
    int saved_fd = dup(stdout_fd);
#endif
    if (saved_fd < 0) {
        return false;
    }

    FILE* tmp = tmpfile();
    if (!tmp) {
#if defined(_WIN32)
        _close(saved_fd);
#else
        close(saved_fd);
#endif
        return false;
    }

    int tmp_fd = fileno(tmp);
#if defined(_WIN32)
    if (_dup2(tmp_fd, stdout_fd) != 0) {
        fclose(tmp);
        _close(saved_fd);
        return false;
    }
#else
    if (dup2(tmp_fd, stdout_fd) < 0) {
        fclose(tmp);
        close(saved_fd);
        return false;
    }
#endif

    fn();
    fflush(stdout);

#if defined(_WIN32)
    _dup2(saved_fd, stdout_fd);
    _close(saved_fd);
#else
    dup2(saved_fd, stdout_fd);
    close(saved_fd);
#endif

    rewind(tmp);
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), tmp)) {
        output += buffer;
    }
    fclose(tmp);
    return true;
}

int main() {
    std::string output;
    bool captured = captureStdout([&]() {
        ChronoLogger logger("DesktopTest", CHRONOLOG_LEVEL_DEBUG);
        logger.debug("Debug message %d", 1);
        logger.info("Info message %s", "OK");
        logger.warn("Warning message");
        logger.error("Error message");
        logger.fatal("Fatal message");
    }, output);

    if (!captured) {
        std::fprintf(stderr, "Failed to capture stdout\n");
        return 1;
    }

    std::vector<const char*> expectations = {
        "Debug message 1",
        "Info message OK",
        "Warning message",
        "Error message",
        "Fatal message"
    };

    for (const char* expected : expectations) {
        if (output.find(expected) == std::string::npos) {
            std::fprintf(stderr, "Missing expected output: %s\n", expected);
            return 2;
        }
    }

    return 0;
}
