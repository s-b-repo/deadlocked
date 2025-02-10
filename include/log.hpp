#pragma once

#include <string>

enum LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

void Log(LogLevel level, std::string message);
