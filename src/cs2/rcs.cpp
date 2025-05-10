#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "mouse.hpp"
#include <chrono>
#include <random>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>          // for glm::length2
#include <glm/gtx/compatibility.hpp> // for glm::lerp
#include <cmath>

glm::vec2 mouse_movement{0.0f};

inline float RandomFloat(float min, float max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

void Rcs() {
    if (!config.aimbot.rcs) {
        return;
    }

    // Rate-limit to ~125Hz (~8ms between updates)
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() < 8) {
        return;
    }
    last_time = now;

    auto local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    WeaponClass wc = local_player->GetWeaponClass();
    if (wc != WeaponClass::Smg && wc != WeaponClass::Rifle && wc != WeaponClass::Heavy) {
        return;
    }

    i32 shots = local_player->ShotsFired();
    if (shots < 1) {
        mouse_movement = glm::vec2{0.0f};
        return;
    }

    // Use length² check for speed and lower threshold
    if (glm::length2(aim_punch) < 0.0001f) {
        return;
    }

    float sens = Sensitivity() * local_player->FovMultiplier();
    glm::vec2 punch_angle{
        aim_punch.y / sens * 25.0f,
       -aim_punch.x / sens * 25.0f
    };

    // Compute delta from what we've already moved
    glm::vec2 delta = punch_angle - mouse_movement;

    // Clamp per-frame so we never move >2px in any direction
    delta = glm::clamp(delta, glm::vec2{-2.0f}, glm::vec2{2.0f});

    // Smooth out the step (50% toward full correction)
    delta = glm::lerp(glm::vec2{0.0f}, delta, 0.5f);

    // Add small random jitter to evade heuristics
    delta.x += RandomFloat(-0.25f, 0.25f);
    delta.y += RandomFloat(-0.25f, 0.25f);

    // Track the total so we can reverse properly next frame
    mouse_movement += delta;

    // Finally, apply mouse move (rounded to int)
    MouseMove(glm::ivec2{
        static_cast<i32>(std::round(delta.x)),
        static_cast<i32>(std::round(delta.y))
    });
}
