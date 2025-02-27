#pragma once

#include <glm/glm.hpp>
#include <string>

#include "types.hpp"

struct PlayerInfo {
    i32 health;
    i32 armor;
    u8 team;
    glm::vec3 position;
    glm::vec3 head;
    f32 rotation;
    bool has_defuser;
    bool has_helmet;
    bool has_bomb;
    bool is_active;
    std::string name;
    u64 steam_id;
    std::string weapon;
    std::vector<std::string> weapons;
    std::vector<std::pair<glm::vec3, glm::vec3>> bones;
};

struct EntityInfo {
    std::string name;
    glm::vec3 position;
};

struct MiscInfo {
    f32 gui_scale = -1.0f;
    std::string held_weapon;
    std::string map_name;
    bool in_game;
    bool is_ffa;
};
