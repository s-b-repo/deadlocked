#pragma once

#include <string>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

void Log(LogLevel level, const std::string &message);
