#pragma once

#include <string>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Off,
};

void Log(LogLevel level, const std::string &message);
