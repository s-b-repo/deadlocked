#pragma once

#include <ios>
#include <sstream>
#include <string>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Off,
};

void Log(LogLevel level, const std::string &message);
void SetLogLevel(LogLevel level);
LogLevel GetLogLevel();
std::string LogLevelString(LogLevel level);

template <typename T>
std::string HexString(T value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    std::stringstream ss;
    ss << std::hex << "0x" << value;
    return ss.str();
}
