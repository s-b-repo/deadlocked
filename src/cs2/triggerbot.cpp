#include <chrono>
#include <random>

#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
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

    if (crosshair_entity->Team() == local_player->Team()) {
        return;
    }

    std::random_device dev;
    std::mt19937 rng{dev()};
    const f32 mean =
        static_cast<f32>(config.triggerbot.delay_min + config.triggerbot.delay_max) / 2.0f;
    std::normal_distribution<f32> normal{
        mean, (config.triggerbot.delay_max - config.triggerbot.delay_min) / 2.0f};

    const i32 delay = static_cast<i32>(normal(rng));
    next_shot = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
}
