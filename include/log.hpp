#pragma once

#include <string>

enum LogLevel {
    Info,
    Warning,
    Error,
};

void Log(LogLevel level, std::string message);
