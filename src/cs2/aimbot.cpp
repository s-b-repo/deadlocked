#include "cs2/aimbot.hpp"

#include "cs2/cs2.hpp"
#include "math.hpp"

void Aimbot() {
    if (!config.aimbot.enabled || !target.player.has_value() || !IsButtonPressed(config.aimbot.hotkey)) {
        return;
    }

    Player target_player = target.player.value();

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
        if (spotted_mask & (1 << target.local_pawn_index) == 0) {
            return;
        }
    }

    glm::vec2 target_angle(0.0);
    if (config.aimbot.multibone) {
        target_angle = target.angle;
    } else {
        const auto head_position = target_player.BonePosition(Bones::BoneHead);
        target_angle = TargetAngle(local_player.EyePosition(), head_position, target.previous_aim_punch);
    }

    const auto view_angles = local_player.ViewAngles();
    if (AnglesToFov(view_angles, target_angle) > (config.aimbot.fov * DistanceScale(target.distance))) {
        return;
    }
}
