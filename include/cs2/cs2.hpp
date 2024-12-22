#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <process.hpp>

#include "cs2/bones.hpp"
#include "cs2/offsets.hpp"

struct PlayerInfo {
    u64 pawn;
    i32 health;
    i32 armor;
    u8 team;
    glm::vec3 position;
    glm::vec3 head;
    bool visible;
    std::string weapon;
    std::vector<std::pair<glm::vec3, glm::vec3>> bones;
};

struct Target {
    u64 pawn;
    PlayerInfo player;
    Bones bone_index;
    glm::vec2 angle;
};

void CS2();
bool IsValid();
void Setup();
void Run();
