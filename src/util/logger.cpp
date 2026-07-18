#include "logger.h"
#include "platform/platform.h"
#include <iostream>

std::vector<LogEntry> Logger::entries;
uint32_t Logger::currentTick = 0;
uint32_t Logger::currentSeed = 0;
std::string Logger::lastWarningMessage = "";
uint32_t Logger::lastWarningTick = 0;

void Logger::Init(uint32_t seed) {
    entries.clear();
    currentTick = 0;
    currentSeed = seed;
    lastWarningMessage = "";
    lastWarningTick = 0;
    Log(LogLevel::Info, "Logger", "System initialized.");
}

void Logger::Log(LogLevel level, const std::string& subsystem, const std::string& message) {
    // Prevent per-frame repetitive warnings (rate limit: once per 60 ticks/1 sec for exact same message)
    if (level == LogLevel::Warning) {
        if (message == lastWarningMessage && (currentTick - lastWarningTick < 60)) {
            return;
        }
        lastWarningMessage = message;
        lastWarningTick = currentTick;
    }

    LogEntry entry;
    entry.level = level;
    entry.subsystem = subsystem;
    entry.message = message;
    entry.tick = currentTick;

    // Fixed-capacity circular buffer behaviour
    if (entries.size() >= MAX_ENTRIES) {
        entries.erase(entries.begin()); // Erase the oldest entry
    }
    entries.push_back(entry);

    // Format log string
    std::string formatted = "[" + LevelToString(level) + "][" + subsystem + "][Tick:" + std::to_string(currentTick) + 
                            "][Seed:" + std::to_string(currentSeed) + "] " + message;

    // Platform log output
    if (level == LogLevel::Error) {
        std::cerr << formatted << std::endl;
    } else {
        std::cout << formatted << std::endl;
    }
}

void Logger::SetTick(uint32_t tick) {
    currentTick = tick;
}

void Logger::SetSeed(uint32_t seed) {
    currentSeed = seed;
}

const std::vector<LogEntry>& Logger::GetEntries() {
    return entries;
}

void Logger::Clear() {
    entries.clear();
}

std::string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
    }
    return "UNKNOWN";
}
