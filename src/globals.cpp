#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <mutex>
#include <vector>

#include "config.hpp"
#include "cs2/info.hpp"
#include "log.hpp"

std::mutex config_lock;
Config config = LoadConfig();

std::mutex vinfo_lock;
std::vector<PlayerInfo> all_player_info;
std::vector<PlayerInfo> enemy_info;
std::vector<EntityInfo> entity_info;
PlayerInfo local_player;
glm::mat4 view_matrix;
glm::ivec4 window_size;
MiscInfo misc_info;
Flags flags;

std::string ConfigPath() {
    // current executable directory
    const auto exe = std::filesystem::canonical("/proc/self/exe");
    const auto exe_path = exe.parent_path();
    return (exe_path / std::filesystem::path("deadlocked.toml")).string();
}

void SaveConfig() {
    // save config in binary format
    std::ofstream file(ConfigPath());
    if (!file.good()) {
        Log(LogLevel::Warning, "config file invalid, cannot save");
        return;
    }

    file << config.to_toml();
}

Config LoadConfig() {
    std::ifstream file(ConfigPath());
    if (!file.good()) {
        Log(LogLevel::Warning, "config file invalid, laoding defaults");
        return {};
    }

    try {
        const auto data = toml::parse(file);
        return Config::from_toml(*data.as_table());
    } catch (toml::parse_error&) {
        Log(LogLevel::Warning, "config file invalid, laoding defaults");
        return {};
    }
}

void ResetConfig() {
    // default initialized
    config = Config();
    SaveConfig();
}
