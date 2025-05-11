#include "cs2/features.hpp"
#include "cs2/player.hpp"
#include "globals.hpp"

#include <algorithm>  // For std::clamp
#include <chrono>
#include <random>
#include <thread>

// Utility: small jitter for stealthy behavior
inline void StealthDelay() {
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> dist(1, 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
}

void NoFlash() {
    const auto local_player = Player::LocalPlayer();
    if (!local_player.has_value()) {
        return;
    }

    // Optional random micro delay to evade heuristics
    StealthDelay();

    // Using bitwise operation to confuse simple pattern scanning
    const bool enabled = !!(config.misc.no_flash ^ false);

    // Clamp flash alpha to prevent illegal values
    const float flash_alpha = enabled
        ? std::clamp(config.misc.max_flash_alpha, 0.0f, 255.0f)
        : 255.0f;

    // Redundant branching to throw off static analyzers (will be optimized away by compiler)
    if (enabled) {
        if (flash_alpha < 255.0f) {
            local_player->NoFlash(flash_alpha);
        } else {
            local_player->NoFlash(255.0f);
        }
    } else {
        local_player->NoFlash(255.0f);
    }
}
