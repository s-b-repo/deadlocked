#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "math.hpp"
#include "mouse.hpp"
#include <chrono>
#include <thread>
#include <random>

float RandFloat(float min, float max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int RandInt(int min, int max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

// Adds jitter and smooths aim like a human
glm::vec2 HumanizedDelta(const glm::vec2& delta, float smooth) {
    float jitter_x = RandFloat(-0.3f, 0.3f);
    float jitter_y = RandFloat(-0.3f, 0.3f);
    float dynamic_smooth = smooth + RandFloat(-0.25f, 0.35f);

    glm::vec2 adjusted {
        (delta.x + jitter_x) / (dynamic_smooth + 1.0f),
        (delta.y + jitter_y) / (dynamic_smooth + 1.0f)
    };
    return adjusted;
}

void Aimbot() {
    if (!config.aimbot.enabled || !target.player || !IsButtonPressed(config.aimbot.hotkey)) return;
    if (!target.player->IsValid()) return;

    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) return;

    if (config.aimbot.flash_check && local_player->IsFlashed()) return;

    if (config.aimbot.visibility_check) {
        if ((target.player->SpottedMask() & (1 << target.local_pawn_index)) == 0) return;
    }

    glm::vec2 target_angle {};
    if (config.aimbot.multibone) {
        target_angle = target.angle;
    } else {
        target_angle = TargetAngle(
            local_player->EyePosition(), target.player->BonePosition(Bones::Head), aim_punch);
    }

    const glm::vec2 view_angles = local_player->ViewAngles();
    if (AnglesToFov(view_angles, target_angle) >
        config.aimbot.fov * DistanceScale(target.distance)) return;

    if (local_player->ShotsFired() < config.aimbot.start_bullet) return;

    glm::vec2 aim_angles = view_angles - target_angle;
    if (aim_angles.y < -180.0f) aim_angles.y += 360.0f;
    else if (aim_angles.y > 180.0f) aim_angles.y -= 360.0f;
    Vec2Clamp(aim_angles);

    const float sensitivity = Sensitivity() * local_player->FovMultiplier();

    glm::vec2 pixel_delta {
        aim_angles.y / sensitivity * 25.0f,
        -aim_angles.x / sensitivity * 25.0f
    };

    glm::vec2 final_move;
    if (!config.aimbot.aim_lock && config.aimbot.smooth > 0.0f) {
        final_move = HumanizedDelta(pixel_delta, config.aimbot.smooth);
    } else {
        final_move = pixel_delta;
    }

    glm::ivec2 final_int {
        static_cast<int>(final_move.x),
        static_cast<int>(final_move.y)
    };

    // Add random sleep to simulate human reaction
    std::this_thread::sleep_for(std::chrono::milliseconds(RandInt(6, 22)));
    MouseMove(final_int);
}
