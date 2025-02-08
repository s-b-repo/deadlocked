#pragma once

// #include "cs2/cs2.hpp"
#include <glm/glm.hpp>
#include <optional>

#include "types.hpp"
#include "weapon_class.hpp"

class Player {
  public:
    u64 controller;
    u64 pawn;

    static std::optional<Player> LocalPlayer();
    static std::optional<Player> Index(u64 index);
    static std::optional<u64> Pawn(u64 controller);

    i32 Health();
    i32 Armor();
    std::string Name();
    u8 Team();
    u8 LifeState();
    std::string WeaponName();
    WeaponClass GetWeaponClass();
    u64 GameSceneNode();
    bool IsDormant();
    glm::vec3 Position();
    glm::vec3 EyePosition();
    glm::vec3 BonePosition(u64 bone_index);
    i32 ShotsFired();
    f32 FovMultiplier();
    u64 SpottedMask();
    std::vector<std::pair<glm::vec3, glm::vec3>> AllBones();
    bool IsValid();
    glm::vec2 ViewAngles();
    glm::vec2 AimPunch();

    bool Equals(Player &other) { return pawn == other.pawn && controller == other.controller; }
};
