#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "mouse.hpp"
#include <chrono>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp> // for glm::lerp

glm::vec2 mouse_movement {0.0f};

inline float RandomFloat(float min, float max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

void Rcs() {
    if (!config.aimbot.rcs) {
        return;
    }

    static auto last_time = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();

    // Ratelimit to ~125 Hz to prevent overcorrection
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() < 8) {
        return;
    }
    last_time = now;

    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    const WeaponClass weapon_class = local_player->GetWeaponClass();
    if (weapon_class != WeaponClass::Smg &&
        weapon_class != WeaponClass::Rifle &&
        weapon_class != WeaponClass::Heavy) {
        return;
    }

    const i32 shots_fired = local_player->ShotsFired();
    if (shots_fired < 1) {
        mouse_movement = glm::vec2 {0.0f};
        return;
    }

    if (glm::length2(aim_punch) < 0.0001f) {  // use length squared for speed
        return;
    }

    const float sensitivity = Sensitivity() * local_player->FovMultiplier();

    // Calculate new compensation vector
    const glm::vec2 punch_angle {
        aim_punch.y / sensitivity * 25.0f,
       -aim_punch.x / sensitivity * 25.0f
    };

    // Difference from previous movement
    glm::vec2 delta = punch_angle - mouse_movement;

    // Clamp to max pixels/frame
    delta = glm::clamp(delta, glm::vec2{-2.0f, -2.0f}, glm::vec2{2.0f, 2.0f});

    // Add smoothing via linear interpolation
    delta = glm::lerp(glm::vec2{0.0f}, delta, 0.5f);

    // Add random jitter to avoid detection patterns
    delta.x += RandomFloat(-0.25f, 0.25f);
    delta.y += RandomFloat(-0.25f, 0.25f);

    // Track total mouse movement to allow proper reversal
    mouse_movement += delta;

    // Apply movement
    MouseMove(glm::ivec2 {
        static_cast<i32>(std::round(delta.x)),
        static_cast<i32>(std::round(delta.y))
    });
}
