#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <cstdint>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

struct LogEntry {
    LogLevel level;
    std::string subsystem;
    std::string message;
    uint32_t tick;
};

class Logger {
public:
    static void Init(uint32_t seed = 0);
    static void Log(LogLevel level, const std::string& subsystem, const std::string& message);
    static void SetTick(uint32_t tick);
    static void SetSeed(uint32_t seed);
    static const std::vector<LogEntry>& GetEntries();
    static void Clear();
    static std::string LevelToString(LogLevel level);
private:
    static const size_t MAX_ENTRIES = 250;
    static std::vector<LogEntry> entries;
    static uint32_t currentTick;
    static uint32_t currentSeed;

    // Rate-limiting repetitive warnings
    static std::string lastWarningMessage;
    static uint32_t lastWarningTick;
};

#endif // LOGGER_H
