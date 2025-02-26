#pragma once

#include <mutex>
#include <shared_mutex>

#include "config.hpp"
#include "cs2/info.hpp"

extern std::mutex config_lock;
extern Config config;

extern std::shared_mutex vinfo_lock;
extern std::vector<PlayerInfo> all_player_info;
extern std::vector<PlayerInfo> enemy_info;
extern std::vector<EntityInfo> entity_info;
extern PlayerInfo local_player;
extern glm::mat4 view_matrix;
extern glm::ivec4 window_size;
extern MiscInfo misc_info;
extern Flags flags;

extern std::string uuid;
extern bool radar_connected;
