#include "mouse.hpp"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <mithril/logging.hpp>
#include <mithril/types.hpp>

i32 mouse = 0;

std::vector<bool> HexToReversedBinary(char hex_char) {
    int value = std::isdigit(hex_char) ? hex_char - '0' :
                (std::isxdigit(hex_char) && std::islower(hex_char)) ? hex_char - 'a' + 10 :
                hex_char - 'A' + 10;

    std::vector<bool> bits(4);
    for (int i = 0; i < 4; ++i) {
        bits[i] = (value >> i) & 1;
    }
    return bits;
}

std::vector<bool> DecodeCapabilities(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        logging::Warning("cannot open mouse capability device file: {}", filename);
        return {};
    }

    std::string content;
    std::getline(file, content);

    std::vector<bool> binary_out;
    int hex_count = 0;

    for (auto it = content.rbegin(); it != content.rend(); ++it) {
        char c = *it;
        if (c == ' ') {
            binary_out.insert(binary_out.end(), 4 * (16 - hex_count), false);
            hex_count = 0;
        } else if (std::isxdigit(c)) {
            const auto bits = HexToReversedBinary(c);
            binary_out.insert(binary_out.end(), bits.begin(), bits.end());
            hex_count++;
        }
    }

    if (hex_count > 0 && hex_count < 16) {
        binary_out.insert(binary_out.end(), 4 * (16 - hex_count), false);
    }

    return binary_out;
}

void MouseInit() {
    for (const auto &entry : std::filesystem::directory_iterator("/dev/input")) {
        if (!entry.is_character_file()) continue;

        const auto &path = entry.path();
        const std::string name = path.filename().string();
        if (name.rfind("event", 0) != 0) continue;

        const std::string rel_caps_path = "/sys/class/input/" + name + "/device/capabilities/rel";
        const auto rel_caps = DecodeCapabilities(rel_caps_path);
        if (rel_caps.size() <= REL_Y || !rel_caps[REL_X] || !rel_caps[REL_Y]) continue;

        const std::string key_caps_path = "/sys/class/input/" + name + "/device/capabilities/key";
        const auto key_caps = DecodeCapabilities(key_caps_path);
        if (key_caps.size() <= BTN_LEFT || !key_caps[BTN_LEFT]) continue;

        const std::string name_path = "/sys/class/input/" + name + "/device/name";
        std::ifstream name_file(name_path);
        if (!name_file) continue;

        std::string device_name;
        std::getline(name_file, device_name);

        mouse = open(path.c_str(), O_WRONLY);
        if (mouse < 0) {
            logging::Error("could not open mouse: {}", path.string());
            if (errno == EACCES) {
                if (const char* user = getlogin(); user) {
                    logging::Error("user '{}' is not in input group. Try: sudo usermod -aG input {}", user, user);
                }
            } else {
                logging::Error("error code: {}", errno);
            }
            std::exit(1);
        }

        logging::Info("found mouse: {} ({})", device_name, name);
        return;
    }

    mouse = open("/dev/null", O_WRONLY);
    logging::Warning("no mouse device found, using /dev/null");
    std::exit(1);
}

void MouseQuit() {
    if (mouse > 0) {
        close(mouse);
    }
}

void MouseMove(const glm::ivec2 &coords) {
    logging::Debug("mouse move: {}/{}", coords.x, coords.y);

    input_event ev {
        .type = EV_REL,
        .code = REL_X,
        .value = coords.x
    };
    write(mouse, &ev, sizeof(ev));

    ev.code = REL_Y;
    ev.value = coords.y;
    write(mouse, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(mouse, &ev, sizeof(ev)) <= 0) {
        logging::Warning("mouse write failed");
    }
}

void MouseLeftPress() {
    logging::Debug("pressed left mouse button");
    input_event ev {
        .type = EV_KEY,
        .code = BTN_LEFT,
        .value = 1
    };
    write(mouse, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(mouse, &ev, sizeof(ev)) <= 0) {
        logging::Warning("mouse write failed");
    }
}

void MouseLeftRelease() {
    logging::Debug("released left mouse button");
    input_event ev {
        .type = EV_KEY,
        .code = BTN_LEFT,
        .value = 0
    };
    write(mouse, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(mouse, &ev, sizeof(ev)) <= 0) {
        logging::Warning("mouse write failed");
    }
}

bool MouseValid() {
    input_event ev {
        .time = {.tv_sec = 0, .tv_usec = 0},
        .type = EV_SYN,
        .code = SYN_REPORT,
        .value = 0
    };
    return write(mouse, &ev, sizeof(ev)) > 0;
}
