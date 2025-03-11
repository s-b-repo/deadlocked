#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <mithril/logging.hpp>
#include <mutex>
#include <vector>

#include "config.hpp"
#include "cs2/info.hpp"

std::mutex config_lock;
Config config = LoadConfig();

std::mutex vinfo_lock;
std::vector<PlayerInfo> player_info {32};
std::vector<EntityInfo> entity_info {128};
PlayerInfo local_player;
glm::mat4 view_matrix;
glm::vec4 window_size;
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
        logging::Warning("config file invalid, cannot save");
        return;
    }

    file << config.to_toml();
}

Config LoadConfig() {
    std::ifstream file(ConfigPath());
    if (!file.good()) {
        logging::Warning("config file invalid, loading defaults");
        return {};
    }

    try {
        const auto data = toml::parse(file);
        return Config::from_toml(*data.as_table());
    } catch (toml::parse_error &) {
        logging::Warning("config file invalid, loading defaults");
        return {};
    }
}

void ResetConfig() {
    // default initialized
    config = Config();
    SaveConfig();
}
