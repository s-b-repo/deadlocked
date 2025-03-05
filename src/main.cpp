#include "globals.hpp"
#include "gui.hpp"
#include "log.hpp"
#include "mouse.hpp"

int main(const int argc, const char *argv[]) {
    for (int i = 0; i < argc; i++) {
        const std::string arg {argv[i]};
        if (arg == "--file-mem") {
            Log(LogLevel::Info, "reading memory from file");
            flags.file_mem = true;
        } else if (arg.rfind("--scale=") != std::string::npos) {
            misc_info.gui_scale = std::stof(arg.substr(8));
        } else if (arg == "--no-visuals") {
            Log(LogLevel::Info, "disabling visuals");
            flags.no_visuals = true;
        } else if (arg == "--verbose" || arg == "-v") {
            const LogLevel level = GetLogLevel();
            if (level > LogLevel::Debug) {
                SetLogLevel(static_cast<LogLevel>(static_cast<i32>(level) - 1));
                Log(LogLevel::Off, "log level set to: " + LogLevelString(GetLogLevel()));
            }
        } else if (arg == "--silent") {
            const LogLevel level = GetLogLevel();
            if (level < LogLevel::Off) {
                SetLogLevel(static_cast<LogLevel>(static_cast<i32>(level) + 1));
                Log(LogLevel::Off, "log level set to: " + LogLevelString(GetLogLevel()));
            }
        }
    }
    Log(LogLevel::Info, "build time: " + std::string(__DATE__) + " " + std::string(__TIME__));
    MouseInit();
    Gui();
}
