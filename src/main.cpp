#include "gui.hpp"
#include "log.hpp"
#include "mouse.hpp"

int main() {
    Log(LogLevel::Info, "build time: " + std::string(__DATE__) + " " + std::string(__TIME__));
    MouseInit();
    Gui();
}
