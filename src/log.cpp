#include "log.hpp"

#include <iostream>

LogLevel filter_level = LogLevel::Info;

void SetLogLevel(LogLevel level) { filter_level = level; }

std::string LogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
    }
    return "?";
}

void Log(LogLevel level, std::string message) {
    if (level < filter_level) {
        return;
    }
    std::cout << "[" << LogLevelString(level) << "] " << message << "\n";
}
