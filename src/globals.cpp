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
std::vector<PlayerInfo> player_info(32);
std::vector<EntityInfo> entity_info(128);
PlayerInfo local_player;
glm::mat4 view_matrix;
glm::vec4 window_size;
MiscInfo misc_info;
Flags flags;

std::string ConfigPath() {
    static const std::string path = [] {
        try {
            auto exe_path = std::filesystem::canonical("/proc/self/exe").parent_path();
            return (exe_path / "deadlocked.toml").string();
        } catch (const std::filesystem::filesystem_error& e) {
            logging::Error("Failed to resolve executable path: {}", e.what());
            return std::string("deadlocked.toml");
        }
    }();
    return path;
}

void SaveConfig() {
    std::scoped_lock lock(config_lock);
    std::ofstream file(ConfigPath(), std::ios::trunc);
    if (!file) {
        logging::Warning("Could not open config file for writing: {}", ConfigPath());
        return;
    }

    try {
        file << config.to_toml();
    } catch (const std::exception& e) {
        logging::Warning("Failed to serialize config: {}", e.what());
    }
}

Config LoadConfig() {
    std::ifstream file(ConfigPath());
    if (!file) {
        logging::Warning("Config file not found or unreadable, loading defaults");
        return Config{};
    }

    try {
        const auto data = toml::parse(file);
        return Config::from_toml(*data.as_table());
    } catch (const toml::parse_error& e) {
        logging::Warning("Config parse error: {}, loading defaults", e.what());
        return Config{};
    }
}

void ResetConfig() {
    {
        std::scoped_lock lock(config_lock);
        config = Config();
    }
    SaveConfig();
}
