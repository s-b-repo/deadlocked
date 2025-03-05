#pragma once

#include <glm/glm.hpp>
#include <optional>

#include "types.hpp"

enum class BombSite {
    A,
    B,
};

class Bomb {
  public:
    u64 entity;

    std::optional<u64> GetEntity() const;
    bool IsPlanted() const;
    bool IsBeingDefused() const;
    BombSite GetBombSite() const;
    f32 BlowTime() const;
    glm::vec3 Position() const;
};
