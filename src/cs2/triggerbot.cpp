#include <chrono>
#include <random>
#include <thread>       // for std::this_thread::sleep_for
#include <algorithm>    // for std::clamp
#include <optional>

#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "math.hpp"
#include "mouse.hpp"

// Preserve your framework’s globals
static std::optional<std::chrono::steady_clock::time_point> next_shot = std::nullopt;
static bool previous_key_state = false;
static bool toggled = false;

// Normal-distribution delay clamped to [min,max]
inline int RandomizedDelay(int min, int max) {
    static thread_local std::mt19937 rng{ std::random_device{}() };
    float mean  = (min + max) * 0.5f;
    float stdev = (max - min) * 0.5f;
    std::normal_distribution<float> dist(mean, stdev);
    int d = static_cast<int>(std::round(dist(rng)));
    return std::clamp(d, min, max);
}

void Triggerbot() {
    // Master enable check
    if (!config.triggerbot.enabled) {
        return;
    }

    // Toggle vs hold
    bool key_state = IsButtonPressed(config.triggerbot.hotkey);
    if (config.triggerbot.toggle_mode) {
        if (key_state && !previous_key_state) {
            toggled = !toggled;
        }
        previous_key_state = key_state;
        misc_info.triggerbot_active = toggled;
    } else {
        misc_info.triggerbot_active = key_state;
    }

    if (!misc_info.triggerbot_active) {
        return;
    }

    // Fire if our timer has elapsed
    if (next_shot) {
        auto now = std::chrono::steady_clock::now();
        if (now > *next_shot) {
            MouseLeftPress();
            // tiny human-like release delay
            std::this_thread::sleep_for(
                std::chrono::milliseconds(RandomizedDelay(1, 3))
            );
            MouseLeftRelease();
            next_shot = std::nullopt;
        }
        return;
    }

    // Must have local player
    auto local_opt = Player::LocalPlayer();
    if (!local_opt) {
        return;
    }
    auto& local = *local_opt;

    // Flash and scope checks
    if (config.triggerbot.flash_check && local.IsFlashed()) {
        return;
    }
    if (config.triggerbot.scope_check &&
        local.GetWeaponClass() == WeaponClass::Sniper &&
        !local.IsScoped()) {
        return;
    }

    // Entity under crosshair
    auto target_opt = local.EntityInCrosshair();
    if (!target_opt) {
        return;
    }
    auto& target = *target_opt;

    // Head-only filter
    if (config.triggerbot.head_only) {
        glm::vec3 head_pos = target.BonePosition(Bones::Head);
        glm::vec3 eye_pos  = local.EyePosition();

        glm::vec2 tgt_ang  = TargetAngle(eye_pos, head_pos, glm::vec2(0.0f));
        glm::vec2 view_ang = local.ViewAngles();
        float    fov_dist  = AnglesToFov(view_ang, tgt_ang);

        constexpr float HEAD_RADIUS = 3.5f;
        float distance        = glm::distance(eye_pos, head_pos);
        float head_radius_fov = (HEAD_RADIUS / distance) * 100.0f;

        if (fov_dist > head_radius_fov) {
            return;
        }
    }

    // Don’t friendly-fire in team modes
    if (!IsFfa() && target.Team() == local.Team()) {
        return;
    }

    // Schedule next shot with randomized delay
    int delay = RandomizedDelay(
        config.triggerbot.delay_min,
        config.triggerbot.delay_max
    );
    next_shot = std::chrono::steady_clock::now()
              + std::chrono::milliseconds(delay);
}
