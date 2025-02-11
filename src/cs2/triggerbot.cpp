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

    if (next_shot.has_value()) {
        const auto time = next_shot.value();
        const auto now = std::chrono::steady_clock::now();
        if (now > time) {
            MouseLeftPress();
            MouseLeftRelease();
            next_shot = std::nullopt;
        }
        return;
    }

    const auto local_player_opt = Player::LocalPlayer();
    if (!local_player_opt.has_value()) {
        return;
    }
    Player local_player = local_player_opt.value();

    if (config.triggerbot.flash_check && local_player.IsFlashed()) {
        return;
    }

    if (!local_player.HasEntityInCrosshair()) {
        return;
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    const i32 mean = (config.triggerbot.delay_min + config.triggerbot.delay_max) / 2;
    std::normal_distribution<f32> normal(mean, (config.triggerbot.delay_max - config.triggerbot.delay_min) / 2);

    const i32 delay = (i32)normal(rng);
    next_shot = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
}
