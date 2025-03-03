#include "cs2/cs2.hpp"
#include "cs2/features.hpp"
#include "math.hpp"
#include "mouse.hpp"

void Aimbot() {
    if (!config.aimbot.enabled || !target.player || !IsButtonPressed(config.aimbot.hotkey)) {
        return;
    }

    if (!target.player->IsValid()) {
        return;
    }

    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    if (config.aimbot.flash_check && local_player->IsFlashed()) {
        return;
    }

    if (config.aimbot.visibility_check) {
        if ((target.player->SpottedMask() & (1 << target.local_pawn_index)) == 0) {
            return;
        }
    }

    glm::vec2 target_angle {};
    if (config.aimbot.multibone) {
        target_angle = target.angle;
    } else {
        target_angle = TargetAngle(
            local_player->EyePosition(), target.player->BonePosition(Bones::Head),
            target.aim_punch);
    }

    const glm::vec2 view_angles = local_player->ViewAngles();
    if (AnglesToFov(view_angles, target_angle) >
        (config.aimbot.fov * DistanceScale(target.distance))) {
        return;
    }

    if (local_player->ShotsFired() < config.aimbot.start_bullet) {
        return;
    }

    glm::vec2 aim_angles {view_angles - target_angle};
    if (aim_angles.y < -180.0f) {
        aim_angles.y += 360.0f;
    }
    Vec2Clamp(aim_angles);

    const f32 sensitivity = Sensitivity() * local_player->FovMultiplier();

    const glm::vec2 xy {aim_angles.y / sensitivity * 25.0f, -aim_angles.x / sensitivity * 25.0f};
    glm::vec2 smooth_angles;
    if (!config.aimbot.aim_lock && config.aimbot.smooth > 0.0f) {
        smooth_angles =
            glm::vec2 {xy.x / (config.aimbot.smooth + 1.0f), xy.y / (config.aimbot.smooth + 1.0f)};
    } else {
        smooth_angles = xy;
    }

    const glm::ivec2 smooth_int(
        static_cast<i32>(smooth_angles.x), static_cast<i32>(smooth_angles.y));
    MouseMove(smooth_int);
}
