#pragma once

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
    static std::optional<u64> ClientEntity(u64 index);

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
    bool IsFlashed();
    void NoFlash(f32 max_alpha);
    void SetFov(i32 fov);
    glm::vec2 ViewAngles();
    glm::vec2 AimPunch();
    bool HasDefuser();
    bool HasHelmet();
    bool HasBomb();

    bool Equals(Player &other) { return pawn == other.pawn && controller == other.controller; }
};
