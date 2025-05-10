#include <csignal>
#include <unistd.h>
#include <sys/prctl.h>
#include <cstring>
#include <random>
#include <string>

#include <mithril/logging.hpp>
#include <mithril/stacktrace.hpp>

#include "globals.hpp"
#include "gui.hpp"
#include "mouse.hpp"

// ————————————————————————————————————————————————————————————————————————
// Generate a random alphanumeric string of given length.
// ————————————————————————————————————————————————————————————————————————
static std::string GenerateRandomName(size_t length = 12) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    thread_local static std::mt19937_64 eng{std::random_device{}()};
    thread_local static std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string s;
    s.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        s += charset[dist(eng)];
    }
    return s;
}

int main(int argc, char* argv[]) {
    // —————————————————————————————————————————
    // Randomize our process name (Linux only)
    // —————————————————————————————————————————
  #if defined(__linux__)
    const std::string rnd = GenerateRandomName();
    // 1) Tell the kernel (shows in `ps axo comm`)
    prctl(PR_SET_NAME, rnd.c_str(), 0, 0, 0);
    // 2) Overwrite argv[0] buffer (affects `ps axo cmd`)
    size_t avail = std::strlen(argv[0]);
    if (rnd.size() <= avail) {
        std::memcpy(argv[0], rnd.c_str(), rnd.size());
        std::memset(argv[0] + rnd.size(), 0, avail - rnd.size());
    }
  #endif

    // —————————————————————————————————————————
    // Install crash handlers
    // —————————————————————————————————————————
    std::signal(SIGILL,  stacktrace::SignalHandler);
    std::signal(SIGABRT, stacktrace::SignalHandler);
    std::signal(SIGSEGV, stacktrace::SignalHandler);

    // —————————————————————————————————————————
    // Command‐line flags (unchanged)
    // —————————————————————————————————————————
    for (int i = 0; i < argc; i++) {
        const std::string arg{argv[i]};
        if (arg == "--file-mem") {
            logging::Info("reading memory from file");
            flags.file_mem = true;
        } else if (arg.rfind("--scale=", 0) == 0) {
            misc_info.gui_scale = std::stof(arg.substr(8));
        } else if (arg == "--no-visuals") {
            logging::Info("disabling visuals");
            flags.no_visuals = true;
        } else if (arg == "--verbose" || arg == "-v") {
            auto level = logging::GetLevel();
            if (level > logging::Level::Debug) {
                logging::SetLevel(static_cast<logging::Level>(static_cast<int>(level) - 1));
                logging::Info("log level set to: {}", logging::LevelName(logging::GetLevel()));
            }
        } else if (arg == "--silent") {
            auto level = logging::GetLevel();
            if (level < logging::Level::Off) {
                logging::SetLevel(static_cast<logging::Level>(static_cast<int>(level) + 1));
                logging::Info("log level set to: {}", logging::LevelName(logging::GetLevel()));
            }
        }
    }

    logging::Info("build time: {} {}", __DATE__, __TIME__);

    MouseInit();
    Gui();

    return 0;
}
