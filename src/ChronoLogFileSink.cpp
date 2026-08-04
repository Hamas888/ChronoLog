/*
  ChronoLogFileSink.cpp
  Desktop rotating per-module file sink.
*/

#include "ChronoLogFileSink.h"

#if CHRONOLOG_SINKS_ENABLE && defined(CHRONOLOG_PLATFORM_DESKTOP)

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>

ChronoLogFileSink::ChronoLogFileSink()
  : retentionDays_(CHRONOLOG_SINK_FILE_RETENTION_DAYS),
    maxFileBytes_(CHRONOLOG_SINK_FILE_MAX_BYTES),
    rotations_(CHRONOLOG_SINK_FILE_ROTATIONS),
    condensed_(CHRONOLOG_SINK_FILE_CONDENSED != 0),
    currentFile_(NULL),
    currentSize_(0),
    threadCount_(0) {
  snprintf(logsDir_, sizeof(logsDir_), "logs");
  currentModule_[0] = '\0';
  mkdir(logsDir_, 0755);   // ensure default logs/ dir exists
}

ChronoLogFileSink::~ChronoLogFileSink() {
  flush();
  if (currentFile_) fclose(currentFile_);
}

ChronoLogFileSink& ChronoLogFileSink::instance() {
  static ChronoLogFileSink s_instance;
  return s_instance;
}

void ChronoLogFileSink::setLogsDir(const char* path) {
  if (!path) return;
  snprintf(logsDir_, sizeof(logsDir_), "%s", path);
  // Ensure the directory exists.
  mkdir(logsDir_, 0755);
}

void ChronoLogFileSink::setRetentionDays(int days) { retentionDays_ = days > 0 ? days : 0; }
void ChronoLogFileSink::setMaxFileBytes(size_t bytes) { maxFileBytes_ = bytes; }
void ChronoLogFileSink::setRotations(int n) { rotations_ = n > 0 ? n : 0; }
void ChronoLogFileSink::setCondensed(bool on) { condensed_ = on; }

const char* ChronoLogFileSink::levelShort(const char* level) const {
  if (strcmp(level, "INFO") == 0) return "I";
  if (strcmp(level, "DEBUG") == 0) return "D";
  if (strcmp(level, "WARN") == 0) return "W";
  if (strcmp(level, "ERROR") == 0) return "E";
  if (strcmp(level, "FATAL") == 0) return "F";
  if (strcmp(level, "PROGRESS") == 0) return "P";
  return "?";
}

void ChronoLogFileSink::checkRetention(FILE* f, const char* path) {
  if (retentionDays_ <= 0) return;
  struct stat st;
  if (stat(path, &st) != 0) return;
  time_t now = time(NULL);
  if (now - st.st_mtime > (time_t)retentionDays_ * 86400L) {
    // File too old - truncate/recreate (currentSize_ 0 => header rewritten).
    freopen(path, "w", f);
    currentSize_ = 0;
  }
}

void ChronoLogFileSink::checkRotation(const char* module) {
  if (maxFileBytes_ == 0 || currentSize_ < maxFileBytes_) return;
  if (currentFile_) fclose(currentFile_);
  currentFile_ = NULL;
  currentSize_ = 0;

  // Rotate: module.N.txt -> module.N+1.txt (ring of `rotations_` backups).
  char oldPath[512], newPath[512];
  for (int i = rotations_ - 1; i >= 0; i--) {
    snprintf(oldPath, sizeof(oldPath), "%s/%s.%d.txt", logsDir_, module, i);
    if (i == rotations_ - 1) {
      remove(oldPath);   // drop the oldest
    } else {
      snprintf(newPath, sizeof(newPath), "%s/%s.%d.txt", logsDir_, module, i + 1);
      rename(oldPath, newPath);
    }
  }
  // Current file (no suffix) becomes .0.txt
  snprintf(oldPath, sizeof(oldPath), "%s/%s.txt", logsDir_, module);
  snprintf(newPath, sizeof(newPath), "%s/%s.0.txt", logsDir_, module);
  rename(oldPath, newPath);
}

void ChronoLogFileSink::writeHeader(FILE* f, const char* module) {
  // Track the bytes written so currentSize_ reflects the file content and the
  // header is never repeated on reopen.
  char buf[128];
  int n = snprintf(buf, sizeof(buf), "%smodule = \"%s\"\n", CHRONOLOG_LOG_SIGNATURE, module);
  fprintf(f, "%s", buf);
  currentSize_ += (size_t)n;

  char tbuf[32];
  time_t now = time(NULL);
  struct tm* t = localtime(&now);
  strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", t);
  n = snprintf(buf, sizeof(buf), "created = %s\n", tbuf);
  fprintf(f, "%s", buf);
  currentSize_ += (size_t)n;
}

void ChronoLogFileSink::ensureOpen(const char* module) {
  // Ensure the target directory exists (mkdir is idempotent).
  mkdir(logsDir_, 0755);
  // Rotate before opening if the current file exceeds the cap.
  checkRotation(module);

  if (currentFile_ && strcmp(currentModule_, module) == 0) {
    return;  // already open for this module
  }
  if (currentFile_) {
    fclose(currentFile_);
    currentFile_ = NULL;
  }
  snprintf(currentModule_, sizeof(currentModule_), "%s", module);
  char path[512];
  snprintf(path, sizeof(path), "%s/%s.txt", logsDir_, module);
  currentFile_ = fopen(path, "a");
  currentSize_ = 0;
  if (currentFile_) {
    // Record initial size (from a previous session) and apply retention.
    struct stat st;
    if (stat(path, &st) == 0) currentSize_ = (size_t)st.st_size;
    checkRetention(currentFile_, path);
    // Write the header only on a fresh (empty) file - never repeat it on reopen.
    if (currentSize_ == 0) writeHeader(currentFile_, module);
  }
}

void ChronoLogFileSink::write(const ChronoLogSinkLine& line) {
  if (!line.module || !line.message) return;
  ensureOpen(line.module);
  if (!currentFile_) return;

  if (condensed_) {
    // Compact: time |I| T<n> | message, with a thread registry.
    const char* tid = "T?";
    if (line.task && line.task[0] != '\0') {
      bool found = false;
      for (int i = 0; i < threadCount_; i++) {
        if (strcmp(threads_[i].name, line.task) == 0) {
          found = true;
          snprintf(threads_[i].name, sizeof(threads_[i].name), "T%d", threads_[i].id);
          tid = threads_[i].name;
          break;
        }
      }
      if (!found && threadCount_ < 32) {
        strncpy(threads_[threadCount_].name, line.task, sizeof(threads_[threadCount_].name) - 1);
        threads_[threadCount_].name[sizeof(threads_[threadCount_].name) - 1] = '\0';
        threads_[threadCount_].id = threadCount_;
        snprintf(threads_[threadCount_].name, sizeof(threads_[threadCount_].name), "T%d", threadCount_);
        tid = threads_[threadCount_].name;
        threadCount_++;
      }
    }
    fprintf(currentFile_, "%s |%s| %s | %s\n",
            line.time ? line.time : "--:--:--",
            levelShort(line.level),
            tid,
            line.message);
  } else {
    // Full-fidelity: time | module | LEVEL | task | message
    fprintf(currentFile_, "%s | %-15s | %-8s | %-16s | %s\n",
            line.time ? line.time : "--:--:--",
            line.module,
            line.level ? line.level : "",
            line.task ? line.task : "",
            line.message);
  }
  currentSize_ += strlen(line.message) + 64;
}

void ChronoLogFileSink::flush() {
  if (currentFile_) fflush(currentFile_);
}

#endif // CHRONOLOG_SINKS_ENABLE && CHRONOLOG_PLATFORM_DESKTOP
