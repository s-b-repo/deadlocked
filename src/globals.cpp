#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

#include "config.hpp"
#include "cs2/cs2.hpp"
#include "log.hpp"

std::mutex config_lock;
Config config = LoadConfig();

std::mutex vinfo_lock;
std::vector<PlayerInfo> player_info;
std::vector<EntityInfo> entity_info;
glm::mat4 view_matrix;
glm::ivec4 window_size;
bool should_quit = false;

Config DefaultConfig() {
    return Config{.aimbot =
                      {
                          .hotkey = KeyCode::Mouse5,
                          .start_bullet = 2,
                          .fov = 2.5f,
                          .smooth = 5.0f,

                          .enabled = true,
                          .aim_lock = false,
                          .visibility_check = true,
                          .multibone = true,
                          .flash_check = true,
                          .rcs = false,
                      },
                  .visuals =
                      {
                          .box_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                          .skeleton_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                          .armor_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f),

                          .overlay_fps = 120,
                          .line_width = 2.0f,

                          .draw_box = DrawStyle::DrawColor,
                          .draw_skeleton = DrawStyle::DrawHealth,
                          .enabled = true,
                          .draw_health = true,
                          .draw_armor = true,
                          .draw_name = true,
                          .draw_weapon = true,
                          .draw_tags = true,
                          .dropped_weapons = true,
                          .debug_window = false,
                      },
                  .misc = {
                      .max_flash_alpha = 0.0f,
                      .desired_fov = 90,

                      .no_flash = false,
                      .fov_changer = false,
                  }};
}

std::string ConfigPath() {
    // current executable directory
    const auto exe = std::filesystem::canonical("/proc/self/exe");
    const auto exe_path = exe.parent_path();
    return (exe_path / std::filesystem::path("deadlocked.json")).string();
}

void SaveConfig() {
    // save config in binary format
    std::ofstream config_file(ConfigPath(), std::ios::binary);
    if (!config_file.good()) {
        Log(LogLevel::Error, "config file is not opened");
        return;
    }

    config_file.write((const char *)(&config), sizeof(Config));
    config_file.seekp(0);
}

Config LoadConfig() {
    // load config in binary format
    Config config = DefaultConfig();
    std::ifstream file(ConfigPath(), std::ios::binary);
    if (!file.good()) {
        return config;
    }

    return config;
}

void ResetConfig() {
    config = DefaultConfig();
    SaveConfig();
}
