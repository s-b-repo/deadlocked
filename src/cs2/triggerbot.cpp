#include <chrono>
#include <random>

#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "math.hpp"
#include "mouse.hpp"

std::optional<std::chrono::steady_clock::time_point> next_shot = std::nullopt;

void Triggerbot() {
    if (!config.triggerbot.enabled || !IsButtonPressed(config.triggerbot.hotkey)) {
        return;
    }

    if (next_shot) {
        const auto time = *next_shot;
        const auto now = std::chrono::steady_clock::now();
        if (now > time) {
            MouseLeftPress();
            MouseLeftRelease();
            next_shot = std::nullopt;
        }
        return;
    }

    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    if (config.triggerbot.flash_check && local_player->IsFlashed()) {
        return;
    }

    if (config.triggerbot.scope_check && local_player->GetWeaponClass() == WeaponClass::Sniper &&
        !local_player->IsScoped()) {
        return;
    }

    const std::optional<Player> crosshair_entity = local_player->EntityInCrosshair();
    if (!crosshair_entity) {
        return;
    }

    if (config.triggerbot.head_only) {
        glm::vec3 enemy_head = target.player->BonePosition(Bones::Head);
        glm::vec3 crosshair_pos = local_player->EyePosition();

        glm::vec2 target_angle = TargetAngle(crosshair_pos, enemy_head, glm::vec2(0.0f));

        glm::vec2 view_angles = local_player->ViewAngles();

        float fov_distance = AnglesToFov(view_angles, target_angle);

        float head_radius_world = 3.5f;
        float head_radius_fov = head_radius_world / target.distance * 100.0f;

        if (fov_distance > head_radius_fov) {
            return;
        }
    }

    if (!IsFfa() && crosshair_entity->Team() == local_player->Team()) {
        return;
    }

    std::random_device dev;
    std::mt19937 rng {dev()};
    const f32 mean =
        static_cast<f32>(config.triggerbot.delay_min + config.triggerbot.delay_max) / 2.0f;
    std::normal_distribution normal {
        mean, static_cast<f32>(config.triggerbot.delay_max - config.triggerbot.delay_min) / 2.0f};

    const i32 delay = static_cast<i32>(normal(rng));
    next_shot = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
}
