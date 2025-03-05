#pragma once

#include "types.hpp"

enum class BombSite {
    A,
    B,
};

class Bomb {
  public:
    u64 entity;

    bool IsPlanted() const;
    bool IsBeingDefused() const;
    BombSite BombSite() const;
    f32 BlowTime() const;
};
