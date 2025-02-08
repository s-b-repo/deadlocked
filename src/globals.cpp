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
    return Config{.aimbot =
                      {
                          .hotkey = KeyCode::Mouse5,
                          .start_bullet = 2,
                          .fov = 2.5,
                          .smooth = 5.0,

                          .enabled = true,
                      },
                  .visuals = {
                      .box_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                      .skeleton_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                      .armor_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f),

                      .overlay_fps = 120,
                      .draw_box = DrawStyle::DrawColor,
                      .draw_skeleton = DrawStyle::DrawHealth,

                      .enabled = true,
                      .draw_health = true,
                      .draw_armor = true,
                      .draw_weapon = false,
                      .debug_window = false,
                  }};
}

void ColorClamp(ImVec4 *color) {
    color->x = glm::clamp(color->x, 0.0f, 1.0f);
    color->y = glm::clamp(color->y, 0.0f, 1.0f);
    color->z = glm::clamp(color->z, 0.0f, 1.0f);
    color->w = glm::clamp(color->w, 0.0f, 1.0f);
}

void ValidateConfig(Config *conf) {
    // aimbot

    // visuals
    if (conf->visuals.draw_box < DrawStyle::DrawNone || conf->visuals.draw_box > DrawStyle::DrawHealth) {
        conf->visuals.draw_box = DrawStyle::DrawColor;
    }
    if (conf->visuals.draw_skeleton < DrawStyle::DrawNone || conf->visuals.draw_skeleton > DrawStyle::DrawHealth) {
        conf->visuals.draw_skeleton = DrawStyle::DrawHealth;
    }
    if (conf->visuals.overlay_fps < 30 || conf->visuals.overlay_fps > 240) {
        conf->visuals.overlay_fps = 120;
    }
    ColorClamp(&conf->visuals.box_color);
    ColorClamp(&conf->visuals.skeleton_color);
    ColorClamp(&conf->visuals.armor_color);
}

std::string ConfigPath() {
    // get the home directory
    const char *home = getenv("HOME");
    if (home == nullptr) {
        return "deadlocked.config";
    }

    return std::string(home) + "/.config/deadlocked.config";
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
