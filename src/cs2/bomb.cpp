#include "cs2/bomb.hpp"

bool Bomb::IsPlanted() const {
    return false;
}

bool Bomb::IsBeingDefused() const {
    return false;
}

BombSite Bomb::BombSite() const {
    return BombSite::A;
}

f32 Bomb::BlowTime() const {
    return 0.0f;
}
