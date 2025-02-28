#pragma once

#include <glm/glm.hpp>
#include <optional>

#include "cs2/bones.hpp"
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

    [[nodiscard]] i32 Health() const;
    [[nodiscard]] i32 Armor() const;
    [[nodiscard]] std::string Name() const;
    [[nodiscard]] u64 SteamID() const;
    [[nodiscard]] u8 Team() const;
    [[nodiscard]] u8 LifeState() const;
    [[nodiscard]] std::string WeaponName() const;
    [[nodiscard]] std::vector<std::string> AllWeapons() const;
    [[nodiscard]] WeaponClass GetWeaponClass() const;
    [[nodiscard]] u64 GameSceneNode() const;
    [[nodiscard]] bool IsDormant() const;
    [[nodiscard]] glm::vec3 Position() const;
    [[nodiscard]] glm::vec3 EyePosition() const;
    [[nodiscard]] glm::vec3 BonePosition(Bones bone_index) const;
    [[nodiscard]] f32 Rotation() const;
    [[nodiscard]] i32 ShotsFired() const;
    [[nodiscard]] f32 FovMultiplier() const;
    [[nodiscard]] u64 SpottedMask() const;
    [[nodiscard]] std::vector<std::pair<glm::vec3, glm::vec3>> AllBones() const;
    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] bool IsFlashed() const;
    void NoFlash(f32 max_alpha) const;
    void SetFov(i32 fov) const;
    [[nodiscard]] glm::vec2 ViewAngles() const;
    [[nodiscard]] glm::vec2 AimPunch() const;
    [[nodiscard]] bool HasDefuser() const;
    [[nodiscard]] bool HasHelmet() const;
    [[nodiscard]] bool HasBomb() const;
    [[nodiscard]] std::optional<u64> SpectatorTarget() const;
    // returns player with pawn only, no controller set
    [[nodiscard]] std::optional<Player> EntityInCrosshair() const;
    [[nodiscard]] bool IsScoped() const;

    [[nodiscard]] bool Equals(const Player &other) const {
        return pawn == other.pawn && controller == other.controller;
    }
};
