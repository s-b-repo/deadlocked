#include "cs2/cs2.hpp"
#include "gui.hpp"
#include "log.hpp"
#include "mouse.hpp"

extern MiscInfo misc_info;
extern bool file_mem;

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        const std::string arg = std::string(argv[i]);
        if (arg == "--file-mem") {
            Log(LogLevel::Info, "reading memory from file");
            file_mem = true;
        } else if (arg.rfind("--scale=") != std::string::npos) {
            misc_info.gui_scale = std::stof(arg.substr(8));
        }
    }
    Log(LogLevel::Info, "build time: " + std::string(__DATE__) + " " + std::string(__TIME__));
    MouseInit();
    Gui();
}
