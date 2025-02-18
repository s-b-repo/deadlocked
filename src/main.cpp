#include "cs2/cs2.hpp"
#include "gui.hpp"
#include "log.hpp"
#include "mouse.hpp"

extern MiscInfo misc_info;
extern Flags flags;

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        const std::string arg = std::string(argv[i]);
        if (arg == "--file-mem") {
            Log(LogLevel::Info, "reading memory from file");
            flags.file_mem = true;
        } else if (arg.rfind("--scale=") != std::string::npos) {
            misc_info.gui_scale = std::stof(arg.substr(8));
        } else if (arg == "--no-visuals") {
            Log(LogLevel::Info, "disabling visuals");
            flags.no_visuals = true;
        }
    }
    Log(LogLevel::Info, "build time: " + std::string(__DATE__) + " " + std::string(__TIME__));
    MouseInit();
    Gui();
}
