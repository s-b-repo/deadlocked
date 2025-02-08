#include "log.hpp"

#include <stdarg.h>

#include <iostream>

std::string LogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
    }
    return "?";
}

void Log(LogLevel level, std::string message) { std::cout << "[" << LogLevelString(level) << "] " << message << "\n"; }
