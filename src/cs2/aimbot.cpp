#include "cs2/aimbot.hpp"

#include "cs2/cs2.hpp"
#include "math.hpp"
#include "mouse.hpp"

void Aimbot() {
    if (!config.aimbot.enabled || !target.player.has_value() || !IsButtonPressed(config.aimbot.hotkey)) {
        return;
    }

    Player target_player = target.player.value();
    if (!target_player.IsValid()) {
        return;
    }

    const auto local_player_opt = Player::LocalPlayer();
    if (!local_player_opt.has_value()) {
        return;
    }
    Player local_player = local_player_opt.value();

    if (config.aimbot.flash_check && local_player.IsFlashed()) {
        return;
    }

    if (config.aimbot.visibility_check) {
        const u64 spotted_mask = target_player.SpottedMask();
        if ((spotted_mask & (1 << target.local_pawn_index)) == 0) {
            return;
        }
    }

    glm::vec2 target_angle(0.0);
    if (config.aimbot.multibone) {
        target_angle = target.angle;
    } else {
        const auto head_position = target_player.BonePosition(Bones::BoneHead);
        target_angle = TargetAngle(local_player.EyePosition(), head_position, target.aim_punch);
    }

    const auto view_angles = local_player.ViewAngles();
    if (AnglesToFov(view_angles, target_angle) > (config.aimbot.fov * DistanceScale(target.distance))) {
        return;
    }

    if (local_player.ShotsFired() < config.aimbot.start_bullet) {
        return;
    }

    auto aim_angles = view_angles - target_angle;
    if (aim_angles.y < -180.0f) {
        aim_angles.y += 360.0f;
    }
    Vec2Clamp(aim_angles);

    const auto sensitivity = Sensitivity() * local_player.FovMultiplier();

    const auto xy = glm::vec2(aim_angles.y / sensitivity * 50.0f, -aim_angles.x / sensitivity * 50.0f);
    glm::vec2 smooth_angles(0.0f);
    if (!config.aimbot.aim_lock && config.aimbot.smooth > 1.0f) {
        smooth_angles = glm::vec2(xy.x / config.aimbot.smooth, xy.y / config.aimbot.smooth);
    } else {
        smooth_angles = xy;
    }

    glm::ivec2 smooth_int((i32)smooth_angles.x, (i32)smooth_angles.y);
    MouseMove(smooth_int);
}
