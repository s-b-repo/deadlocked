#include "mouse.hpp"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "log.hpp"
#include "types.hpp"

i32 mouse = 0;

// Helper function that converts a hex digit to its 4-bit reversed binary string.
// For example, for '2': its normal binary is "0010", and reversed is "0100".
std::string HexToReversedBinary(char hex_char) {
    int value;
    if (hex_char >= '0' && hex_char <= '9')
        value = hex_char - '0';
    else if (hex_char >= 'a' && hex_char <= 'f')
        value = hex_char - 'a' + 10;
    else if (hex_char >= 'A' && hex_char <= 'F')
        value = hex_char - 'A' + 10;
    else
        return "";  // invalid hex digit

    // Build the 4-bit binary in reversed order:
    // Normally the bits (from MSB to LSB) would be bit3,bit2,bit1,bit0.
    // We need to append bits in order: bit0, bit1, bit2, bit3.
    std::string reversed_bits;
    for (int i = 0; i < 4; ++i) {
        // Extract bit i (starting from LSB)
        reversed_bits.push_back(((value >> i) & 1) ? '1' : '0');
    }
    return reversed_bits;
}

// Function that reads an input file and decodes its evdev capabilities.
// The algorithm works by reading the file content in reverse order,
// handling hex groups and inserting padding for groups with less than 16 hex characters.
std::string DecodeCapabilities(const std::string &filename) {
    // Read the entire file into a string.
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string content = buffer.str();

    std::string binaryOutput;
    int hexCount = 0;  // counter for number of hex digits in current group

    // Process the file content in reverse order.
    for (auto it = content.rbegin(); it != content.rend(); ++it) {
        char c = *it;
        if (c == '\n') {
            // Ignore linefeed characters.
            continue;
        } else if (c == ' ') {
            // When a space is encountered, this means the current group ended.
            // If the group is less than 16 hex digits, we need to pad with false bits (zeros).
            int padBits = 4 * (16 - hexCount);
            binaryOutput.append(padBits, '0');
            hexCount = 0;  // reset for next group.
        } else if (std::isxdigit(c)) {
            // c is a hex digit.
            std::string bits = HexToReversedBinary(c);
            binaryOutput.append(bits);
            hexCount++;
        }
        // Other characters are not expected.
    }
    // After processing, if the last group (at the beginning of the file) is less than 16 hex
    // digits, pad it.
    if (hexCount > 0 && hexCount < 16) {
        int padBits = 4 * (16 - hexCount);
        binaryOutput.append(padBits, '0');
    }

    return binaryOutput;
}

// Example usage:
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: decode <input_file>" << std::endl;
        return 1;
    }
    std::string decoded = DecodeCapabilities(argv[1]);
    std::cout << "Decoded binary data:\n" << decoded << std::endl;
    return 0;
}

void MouseInit() {
    for (const auto &entry : std::filesystem::directory_iterator("/dev/input")) {
        if (!entry.is_character_file()) {
            continue;
        }

        const std::string event_name = entry.path().filename().string();
        if (event_name.rfind("event", 0) != 0) {
            continue;
        }

        const std::string path {"/sys/class/input/" + event_name + "/device/capabilities/rel"};
        std::ifstream rel_file(path);
        if (!rel_file.is_open()) {
            continue;
        }

        std::string hex_str;
        rel_file >> hex_str;
        rel_file.close();

        u64 caps = 0;
        std::stringstream hex_conversion;
        hex_conversion << std::hex << hex_str;
        hex_conversion >> caps;

        // Check whether the REL_X (bit 0) and REL_Y (bit 1) bits are set.
        bool has_rel_x = (caps & (1 << REL_X)) != 0;
        bool has_rel_y = (caps & (1 << REL_Y)) != 0;

        if (!has_rel_x || !has_rel_y) {
            continue;
        }

        const std::string key_path {"/sys/class/input" + event_name + "/device/capabilities/key"};

        const std::string name_path {"/sys/class/input/" + event_name + "/device/name"};
        std::ifstream name_file(name_path);
        if (!name_file.is_open()) {
            continue;
        }

        std::string device_name;
        std::getline(name_file, device_name);
        name_file.close();

        mouse = open(entry.path().c_str(), O_WRONLY);
        if (mouse < 0) {
            Log(LogLevel::Error, "could not open mouse");
            const auto error = errno;
            if (error == EACCES) {
                const std::string username = getlogin();
                Log(LogLevel::Error,
                    "user is not in input group. please execute sudo usermod -aG input " +
                        username);
            } else {
                Log(LogLevel::Error, "error code: " + std::to_string(error));
            }
            std::exit(1);
        }

        Log(LogLevel::Info, "found mouse: " + device_name + " (" + event_name + ")");
        return;
    }

    mouse = open("/dev/null", O_WRONLY);
    Log(LogLevel::Warning, "no mouse device was found");
    std::exit(1);
}

void MouseMove(const glm::ivec2 &coords) {
    Log(LogLevel::Debug,
        "mouse move: " + std::to_string(coords.x) + "/" + std::to_string(coords.y));
    struct input_event ev {};

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

void MouseLeftPress() {
    Log(LogLevel::Debug, "pressed left mouse button");
    struct input_event ev {};

    // y
    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = 1;
    write(mouse, &ev, sizeof(ev));

    // syn
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(mouse, &ev, sizeof(ev));
}

void MouseLeftRelease() {
    Log(LogLevel::Debug, "released left mouse button");
    struct input_event ev {};

    // y
    ev.type = EV_KEY;
    ev.code = BTN_LEFT;
    ev.value = 0;
    write(mouse, &ev, sizeof(ev));

    // syn
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(mouse, &ev, sizeof(ev));
}
