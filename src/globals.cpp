#include <fstream>
#include <glm/glm.hpp>
#include <vector>

#include "config.hpp"
#include "cs2/cs2.hpp"

Config LoadConfig();

Config config = LoadConfig();
std::vector<PlayerInfo> player_info;
glm::mat4 view_matrix;
glm::ivec4 window_size;

Config DefaultConfig() {
    return Config{
        .box_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
        .skeleton_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
        .armor_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f),

        .overlay_fps = 120,
        .draw_box = DrawStyle::DrawColor,
        .draw_skeleton = DrawStyle::DrawHealth,

        .visuals_enabled = true,
        .draw_health = true,
        .draw_armor = true,
        .draw_weapon = false,
        .visibility_check = false,
        .debug_window = false,
    };
}

void ColorClamp(ImVec4 *color) {
    color->x = glm::clamp(color->x, 0.0f, 1.0f);
    color->y = glm::clamp(color->y, 0.0f, 1.0f);
    color->z = glm::clamp(color->z, 0.0f, 1.0f);
    color->w = glm::clamp(color->w, 0.0f, 1.0f);
}

void ValidateConfig(Config *conf) {
    if (conf->draw_box < DrawStyle::DrawNone || conf->draw_box > DrawStyle::DrawHealth) {
        conf->draw_box = DrawStyle::DrawColor;
    }
    if (conf->draw_skeleton < DrawStyle::DrawNone || conf->draw_skeleton > DrawStyle::DrawHealth) {
        conf->draw_skeleton = DrawStyle::DrawHealth;
    }
    if (conf->overlay_fps < 30 || conf->overlay_fps > 240) {
        conf->overlay_fps = 120;
    }
    ColorClamp(&conf->box_color);
    ColorClamp(&conf->skeleton_color);
    ColorClamp(&conf->armor_color);
}

std::string ConfigPath() {
    // get the home directory
    const char *home = getenv("HOME");
    if (home == nullptr) {
        return "deadlocked.bin";
    }

    return std::string(home) + "/.config/deadlocked.bin";
}

std::ofstream config_file(ConfigPath(), std::ios::binary);

void SaveConfig() {
    // save config in binary format
    if (!config_file.is_open()) {
        return;
    }

    config_file.write((const char *)(&config), sizeof(Config));
    config_file.seekp(0);
}

Config LoadConfig() {
    // load config in binary format
    Config conf = DefaultConfig();
    std::ifstream file(ConfigPath(), std::ios::binary);
    if (!file.is_open()) {
        return conf;
    }

    file.read((char *)(&conf), sizeof(Config));
    file.close();

    ValidateConfig(&conf);

    return conf;
}

void ResetConfig() {
    config = DefaultConfig();
    SaveConfig();
}
