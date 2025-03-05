#include "cs2/bomb.hpp"

#include "cs2/cs2.hpp"

std::optional<u64> Bomb::GetEntity() const {
    const u64 bomb_handle = process.Read<u64>(offsets.direct.planted_c4);
    if (!bomb_handle) {
        return std::nullopt;
    }

    const u64 bomb = process.Read<u64>(bomb_handle);
    if (!bomb) {
        return std::nullopt;
    }

    return bomb;
}

bool Bomb::IsPlanted() const {
    const auto bomb = GetEntity();
    if (!bomb) {
        return false;
    }
    return process.Read<bool>(*bomb + offsets.planted_c4.is_activated);
}

bool Bomb::IsBeingDefused() const {
    const auto bomb = GetEntity();
    if (!bomb) {
        return false;
    }
    return process.Read<bool>(*bomb + offsets.planted_c4.being_defused);
}

BombSite Bomb::GetBombSite() const { return BombSite::A; }

f32 Bomb::BlowTime() const { return 0.0f; }

glm::vec3 Bomb::Position() const {
    const Player bomb_entity {.controller = 0, .pawn = entity};
    return bomb_entity.Position();
}
