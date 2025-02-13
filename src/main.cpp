#include "gui.hpp"
#include "log.hpp"
#include "mouse.hpp"

extern bool file_mem;

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        const char *arg = argv[i];
        if (std::string(arg) == "--file-mem") {
            Log(LogLevel::Info, "reading memory from file");
            file_mem = true;
        }
    }
    Log(LogLevel::Info, "build time: " + std::string(__DATE__) + " " + std::string(__TIME__));
    MouseInit();
    Gui();
}
