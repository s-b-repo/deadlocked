#include "mouse.hpp"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include "log.hpp"

i32 mouse = 0;

void MouseInit() {
    for (const auto &entry : std::filesystem::directory_iterator("/dev/input")) {
        if (!entry.is_character_file()) {
            continue;
        }

        const auto event_name = entry.path().filename().string();
        if (event_name.rfind("event", 0) != 0) {
            continue;
        }

        const auto path = "/sys/class/input/" + event_name + "/device/capabilities/rel";
        std::ifstream rel_file(path);
        if (!rel_file.is_open()) {
            continue;
        }

        std::string hex_str;
        rel_file >> hex_str;
        rel_file.close();

        u64 caps = 0;
        std::stringstream ss;
        ss << std::hex << hex_str;
        ss >> caps;

        // Check whether the REL_X (bit 0) and REL_Y (bit 1) bits are set.
        bool has_rel_x = (caps & (1 << REL_X)) != 0;
        bool has_rel_y = (caps & (1 << REL_Y)) != 0;

        if (!has_rel_x || !has_rel_y) {
            continue;
        }

        std::string device_name;
        const auto name_path = "/sys/class/input/" + event_name + "/device/name";
        std::ifstream name_file(name_path);
        if (!name_file.is_open()) {
            continue;
        }

        std::getline(name_file, device_name);
        name_file.close();

        mouse = open(entry.path().c_str(), O_WRONLY);

        Log(LogLevel::Info, "found mouse: " + device_name);
        return;
    }

    mouse = open("/dev/null", O_WRONLY);
    Log(LogLevel::Warning, "no mouse was found");
}

void MouseMove(glm::ivec2 coords) {
    struct input_event ev = {0};

    // x
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = coords.x;
    write(mouse, &ev, sizeof(ev));

    // y
    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = coords.y;
    write(mouse, &ev, sizeof(ev));

    // syn
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(mouse, &ev, sizeof(ev));
}
