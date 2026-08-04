/*
  ChronoLogLittleFsSink.cpp
  ESP-IDF + Arduino-ESP32 LittleFS file sink with optional UART fetch responder.
*/

#include "ChronoLogLittleFsSink.h"

#if CHRONOLOG_SINKS_ENABLE && (defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP))

#include <sys/stat.h>
#include <errno.h>

#if defined(CHRONOLOG_PLATFORM_ESP_IDF)
  #include "esp_littlefs.h"
  #include "esp_vfs_fs.h"
  #define CHRONO_FS_MOUNT_POINT "/spiffs"
#else
  #include <LittleFS.h>
  #include <FS.h>
  #define CHRONO_FS_MOUNT_POINT ""
#endif

// --- File abstraction: a tiny struct wrapping either FILE* or Arduino File. ---
#if defined(CHRONOLOG_PLATFORM_ESP_IDF)
struct chrono_file {
  FILE* f;
  bool isOpen;
};
#else
struct chrono_file {
  File f;
  bool isOpen;
};
#endif

static chrono_file chrono_open(const char* path, const char* mode) {
  chrono_file r; r.isOpen = false;
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    r.f = fopen(path, mode);
    r.isOpen = (r.f != NULL);
  #else
    if (!LittleFS.begin(true)) return r;
    char full[256];
    if (path[0] != '/') snprintf(full, sizeof(full), "/%s", path); else snprintf(full, sizeof(full), "%s", path);
    r.f = LittleFS.open(full, (mode[0] == 'a') ? FILE_APPEND : (mode[0] == 'r') ? "r" : FILE_WRITE);
    r.isOpen = (bool)r.f;
  #endif
  return r;
}

static size_t chrono_write(chrono_file* f, const char* s, size_t n) {
  if (!f || !f->isOpen) return 0;
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    return fwrite(s, 1, n, f->f);
  #else
    return f->f.write((const uint8_t*)s, n);
  #endif
}

static size_t chrono_read(chrono_file* f, char* buf, size_t n) {
  if (!f || !f->isOpen) return 0;
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    return fread(buf, 1, n, f->f);
  #else
    if (!f->f.available()) return 0;
    return f->f.readBytes(buf, n);
  #endif
}

static void chrono_close(chrono_file* f) {
  if (f && f->isOpen) {
    #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
      fclose(f->f);
    #else
      f->f.close();
    #endif
    f->isOpen = false;
  }
}

static void chrono_flush(chrono_file* f) {
  if (f && f->isOpen) {
    #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
      fflush(f->f);
    #else
      f->f.flush();
    #endif
  }
}

static int chrono_remove_path(const char* path) {
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    return remove(path);
  #else
    if (!LittleFS.begin(true)) return -1;
    char full[256];
    if (path[0] != '/') snprintf(full, sizeof(full), "/%s", path); else snprintf(full, sizeof(full), "%s", path);
    return LittleFS.remove(full) ? 0 : -1;
  #endif
}

static int chrono_rename_path(const char* a, const char* b) {
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    return rename(a, b);
  #else
    if (!LittleFS.begin(true)) return -1;
    char fa[256], fb[256];
    if (a[0] != '/') snprintf(fa, sizeof(fa), "/%s", a); else snprintf(fa, sizeof(fa), "%s", a);
    if (b[0] != '/') snprintf(fb, sizeof(fb), "/%s", b); else snprintf(fb, sizeof(fb), "%s", b);
    return LittleFS.rename(fa, fb) ? 0 : -1;
  #endif
}

static long chrono_size_of(const char* path) {
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
  #else
    if (!LittleFS.begin(true)) return -1;
    char full[256];
    if (path[0] != '/') snprintf(full, sizeof(full), "/%s", path); else snprintf(full, sizeof(full), "%s", path);
    if (!LittleFS.exists(full)) return -1;
    File f = LittleFS.open(full, "r");
    if (!f) return -1;
    long sz = (long)f.size();
    f.close();
    return sz;
  #endif
}

ChronoLogLittleFsSink::ChronoLogLittleFsSink()
  : mounted_(false),
    maxFileBytes_(CHRONOLOG_SINK_FILE_MAX_BYTES),
    rotations_(CHRONOLOG_SINK_FILE_ROTATIONS),
    condensed_(CHRONOLOG_SINK_FILE_CONDENSED != 0),
    timeProvider_(NULL),
    currentSize_(0) {
  currentModule_[0] = '\0';
}

ChronoLogLittleFsSink::~ChronoLogLittleFsSink() {
  flush();
}

ChronoLogLittleFsSink& ChronoLogLittleFsSink::instance() {
  static ChronoLogLittleFsSink s_instance;
  return s_instance;
}

bool ChronoLogLittleFsSink::begin() {
  if (mounted_) return true;
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    esp_vfs_littlefs_conf_t conf = {
      .base_path = CHRONO_FS_MOUNT_POINT,
      .partition_label = NULL,
      .format_if_mount_failed = true,
      .dont_mount = false,
    };
    mounted_ = (esp_vfs_littlefs_register(&conf) == ESP_OK);
  #else
    mounted_ = LittleFS.begin(true);   // format-on-fail once
  #endif
  return mounted_;
}

void ChronoLogLittleFsSink::setMaxFileBytes(size_t bytes) { maxFileBytes_ = bytes; }
void ChronoLogLittleFsSink::setRotations(int n) { rotations_ = n > 0 ? n : 0; }
void ChronoLogLittleFsSink::setCondensed(bool on) { condensed_ = on; }
void ChronoLogLittleFsSink::setTimeProvider(ChronoLogTimeProvider p) { timeProvider_ = p; }

const char* ChronoLogLittleFsSink::levelShort(const char* level) const {
  if (strcmp(level, "INFO") == 0) return "I";
  if (strcmp(level, "DEBUG") == 0) return "D";
  if (strcmp(level, "WARN") == 0) return "W";
  if (strcmp(level, "ERROR") == 0) return "E";
  if (strcmp(level, "FATAL") == 0) return "F";
  if (strcmp(level, "PROGRESS") == 0) return "P";
  return "?";
}

void ChronoLogLittleFsSink::writeHeader(void* file, const char* module) {
  char buf[128];
  int n = snprintf(buf, sizeof(buf), "%smodule = \"%s\"\n", CHRONOLOG_LOG_SIGNATURE, module);
  chrono_write((chrono_file*)file, buf, (size_t)n);
  currentSize_ += (size_t)n;
}

void ChronoLogLittleFsSink::checkRotation(const char* module) {
  if (maxFileBytes_ == 0 || currentSize_ < maxFileBytes_) return;
  char oldPath[256], newPath[256];
  for (int i = rotations_ - 1; i >= 0; i--) {
    snprintf(oldPath, sizeof(oldPath), "%s/logs/%s.%d.log", CHRONO_FS_MOUNT_POINT, module, i);
    if (i == rotations_ - 1) {
      chrono_remove_path(oldPath);
    } else {
      snprintf(newPath, sizeof(newPath), "%s/logs/%s.%d.log", CHRONO_FS_MOUNT_POINT, module, i + 1);
      chrono_rename_path(oldPath, newPath);
    }
  }
  snprintf(oldPath, sizeof(oldPath), "%s/logs/%s.log", CHRONO_FS_MOUNT_POINT, module);
  snprintf(newPath, sizeof(newPath), "%s/logs/%s.0.log", CHRONO_FS_MOUNT_POINT, module);
  chrono_rename_path(oldPath, newPath);
  currentSize_ = 0;
}

void ChronoLogLittleFsSink::writeFileInternal(const char* module, const char* line) {
  if (!module || !line) return;
  if (!mounted_ && !begin()) return;
  checkRotation(module);

  char path[256];
  snprintf(path, sizeof(path), "%s/logs/%s.log", CHRONO_FS_MOUNT_POINT, module);
  chrono_file f = chrono_open(path, "a");
  if (!f.isOpen) return;

  // Write the module header only on a fresh (empty) file - never repeat on reopen.
  long existing = chrono_size_of(path);
  if (existing <= 0) {
    writeHeader(&f, module);
    snprintf(currentModule_, sizeof(currentModule_), "%s", module);
  }

  chrono_write(&f, line, strlen(line));
  chrono_flush(&f);
  chrono_close(&f);
  currentSize_ += strlen(line) + 64;
}

void ChronoLogLittleFsSink::write(const ChronoLogSinkLine& line) {
  if (!line.module || !line.message) return;
  char buf[CHRONOLOG_BUFFER_LEN + 64];
  if (condensed_) {
    snprintf(buf, sizeof(buf), "%s |%s| %s\n",
             line.time ? line.time : "--:--:--",
             levelShort(line.level),
             line.message);
  } else {
    snprintf(buf, sizeof(buf), "%s | %-15s | %-8s | %-16s | %s\n",
             line.time ? line.time : "--:--:--",
             line.module,
             line.level ? line.level : "",
             line.task ? line.task : "",
             line.message);
  }
  writeFileInternal(line.module, buf);
}

void ChronoLogLittleFsSink::flush() {}

void ChronoLogLittleFsSink::sendFileOverUart(const char* module) {
  char path[256];
  snprintf(path, sizeof(path), "%s/logs/%s.log", CHRONO_FS_MOUNT_POINT, module);
  chrono_file f = chrono_open(path, "r");
  if (!f.isOpen) return;
  char buf[256];
  size_t n;
  // Stream the file contents over the platform output chain.
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF)
    while ((n = chrono_read(&f, buf, sizeof(buf))) > 0) {
      // TODO: emit bytes over the configured UART (HAL_UART / printf)
    }
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP)
    while ((n = chrono_read(&f, buf, sizeof(buf))) > 0) {
      Serial.write((const uint8_t*)buf, n);
    }
  #endif
  chrono_close(&f);
}

void ChronoLogLittleFsSink::pollFetch() {
  // UART fetch responder: reads a command byte; if it matches 'F', stream logs.
  #if defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP)
    while (Serial.available()) {
      char c = (char)Serial.read();
      if (c == 'F') {
        sendFileOverUart(currentModule_);
      }
    }
  #endif
}

#endif // CHRONOLOG_SINKS_ENABLE && (ESP_IDF || ESP)
