// File sink test - verifies the desktop rotating per-module file sink.
// Asserts: logs/ files are created, contain the signature header, full-fidelity
// by default, condensed format when toggled, and retention/rotation works.

#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <sys/stat.h>

#include "../src/ChronoLog.h"
#include "../src/ChronoLogSink.h"
#include "../src/ChronoLogFileSink.h"

static std::string readFile(const char* path) {
  std::ifstream in(path);
  std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  return s;
}

static bool fileExists(const char* path) {
  struct stat st;
  return stat(path, &st) == 0;
}

int main() {
  // Use a temp logs dir to avoid clobbering real logs.
  const char* logsDir = "logs_test";
  #if defined(_WIN32)
    _mkdir(logsDir);
  #else
    mkdir(logsDir, 0755);
  #endif

  ChronoLogSinkRegistry::instance().add(&ChronoLogFileSink::instance());
  ChronoLogFileSink::instance().setLogsDir(logsDir);
  ChronoLogFileSink::instance().setMaxFileBytes(0);
  ChronoLogFileSink::instance().setCondensed(false);   // full-fidelity default

  ChronoLogger logger("FileSinkTest", CHRONOLOG_LEVEL_DEBUG);
  logger.info("hello sink %d", 1);
  logger.debug("debug line");
  ChronoLogSinkRegistry::instance().flushAll();

  char path[512];
  snprintf(path, sizeof(path), "%s/FileSinkTest.txt", logsDir);
  if (!fileExists(path)) {
    std::fprintf(stderr, "FAIL: %s not created\n", path);
    return 1;
  }
  std::string content = readFile(path);
  if (content.find("# chrono-log sig") == std::string::npos) {
    std::fprintf(stderr, "FAIL: missing signature in %s\n", path);
    return 2;
  }
  if (content.find("module = \"FileSinkTest\"") == std::string::npos) {
    std::fprintf(stderr, "FAIL: missing module header\n");
    return 3;
  }
  if (content.find("hello sink 1") == std::string::npos ||
      content.find("debug line") == std::string::npos) {
    std::fprintf(stderr, "FAIL: missing log lines\n");
    return 4;
  }
  // Full-fidelity default: the level should be unabbreviated.
  if (content.find("| INFO |") == std::string::npos) {
    std::fprintf(stderr, "FAIL: expected full-fidelity INFO line, got:\n%s\n", content.c_str());
    return 5;
  }

  // Condensed toggle.
  ChronoLogFileSink::instance().setCondensed(true);
  logger.warn("condensed warn");
  ChronoLogSinkRegistry::instance().flushAll();
  content = readFile(path);
  if (content.find("|W| condensed warn") == std::string::npos) {
    std::fprintf(stderr, "FAIL: expected condensed |W| line, got:\n%s\n", content.c_str());
    return 6;
  }

  // Retention: fake an old file, then write -> should truncate/recreate.
  {
    char p2[512];
    snprintf(p2, sizeof(p2), "%s/Retention.txt", logsDir);
    std::ofstream f(p2); f << "old\n"; f.close();
    // Fake mtime 8 days ago.
    time_t old = time(NULL) - 8 * 86400;
    struct stat st;
    stat(p2, &st);
    #if defined(_WIN32)
      (void)old; (void)st;
    #else
      struct timespec ts[2];
      ts[0] = st.st_atim; ts[1].tv_sec = old; ts[1].tv_nsec = 0;
      utimensat(0, p2, ts, 0);
    #endif
  }
  ChronoLogFileSink::instance().setRetentionDays(7);
  ChronoLogger ret("Retention", CHRONOLOG_LEVEL_INFO);
  ret.info("fresh line after retention");
  ChronoLogSinkRegistry::instance().flushAll();
  {
    char p2[512];
    snprintf(p2, sizeof(p2), "%s/Retention.txt", logsDir);
    std::string c2 = readFile(p2);
    if (c2.find("fresh line after retention") == std::string::npos ||
        c2.find("old") != std::string::npos) {
      std::fprintf(stderr, "FAIL: retention did not truncate old file\n");
      return 7;
    }
  }

  // Cleanup: reset sink so it doesn't hold a file open across tests.
  ChronoLogSinkRegistry::instance().remove(&ChronoLogFileSink::instance());

  std::printf("PASS: file sink tests\n");
  return 0;
}
