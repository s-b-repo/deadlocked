#include <chrono>
#include <random>
#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "math.hpp"
#include "mouse.hpp"

std::optional<std::chrono::steady_clock::time_point> next_shot = std::nullopt;
bool previous_key_state = false;
bool toggled = false;

inline i32 RandomizedDelay(i32 min, i32 max) {
    static thread_local std::mt19937 rng{std::random_device{}()};
    const f32 mean = static_cast<f32>(min + max) / 2.0f;
    const f32 stdev = static_cast<f32>(max - min) / 2.0f;
    std::normal_distribution<f32> dist(mean, stdev);
    i32 delay = std::clamp(static_cast<i32>(dist(rng)), min, max);
    return delay;
}

inline bool CheckHeadOnly(const Player& local, const Player& enemy) {
    const glm::vec3 head_pos = enemy.BonePosition(Bones::Head);
    const glm::vec3 eye_pos = local.EyePosition();
    const glm::vec2 view_angle = local.ViewAngles();
    const glm::vec2 aim_angle = TargetAngle(eye_pos, head_pos, glm::vec2(0.0f));

    const f32 fov_dist = AnglesToFov(view_angle, aim_angle);
    constexpr f32 head_radius_world = 3.5f;
    const f32 head_radius_fov = head_radius_world / target.distance * 100.0f;

    return fov_dist <= head_radius_fov;
}

void Triggerbot() {
    if (!config.triggerbot.enabled) return;

    const bool key_state = IsButtonPressed(config.triggerbot.hotkey);
    if (config.triggerbot.toggle_mode) {
        if (key_state && !previous_key_state) toggled = !toggled;
        previous_key_state = key_state;
        misc_info.triggerbot_active = toggled;
    } else {
        misc_info.triggerbot_active = key_state;
    }

    if (!misc_info.triggerbot_active) return;

    if (next_shot) {
        if (std::chrono::steady_clock::now() > *next_shot) {
            MouseLeftPress();
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + rand() % 3)); // mimic human release
            MouseLeftRelease();
            next_shot = std::nullopt;
        }
        return;
    }

    const auto local = Player::LocalPlayer();
    if (!local || (config.triggerbot.flash_check && local->IsFlashed())) return;

    if (config.triggerbot.scope_check &&
        local->GetWeaponClass() == WeaponClass::Sniper &&
        !local->IsScoped()) return;

    const auto entity = local->EntityInCrosshair();
    if (!entity || (!IsFfa() && entity->Team() == local->Team())) return;

    if (config.triggerbot.head_only && !CheckHeadOnly(*local, *entity)) return;

    next_shot = std::chrono::steady_clock::now() + std::chrono::milliseconds(
        RandomizedDelay(config.triggerbot.delay_min, config.triggerbot.delay_max));
}
