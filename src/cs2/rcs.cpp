#include "cs2/rcs.hpp"

#include "cs2/cs2.hpp"
#include "mouse.hpp"

glm::vec2 previous_aim_punch(0.0);

void Rcs() {
    if (!config.aimbot.rcs) {
        return;
    }

    const auto local_player_opt = Player::LocalPlayer();
    if (!local_player_opt.has_value()) {
        return;
    }
    Player local_player = local_player_opt.value();

    const WeaponClass weapon_class = local_player.GetWeaponClass();
    if (weapon_class != WeaponClass::Smg && weapon_class != WeaponClass::Rifle && weapon_class != WeaponClass::Heavy) {
        return;
    }

    const i32 shots_fired = local_player.ShotsFired();

    if (shots_fired < 1) {
        previous_aim_punch = glm::vec2(0.0f);
        return;
    }

    const f32 sensitivity = Sensitivity() * local_player.FovMultiplier();
    const glm::vec2 xy = glm::vec2(target.aim_punch - previous_aim_punch) * glm::vec2(-0.5f);

    glm::vec2 mouse_angle = glm::vec2(((xy.y * 2.0f) / sensitivity) / -0.022f, ((xy.x * 2.0f) / sensitivity) / 0.022f);

    previous_aim_punch = target.aim_punch;

    MouseMove(glm::ivec2((i32)mouse_angle.x, (i32)mouse_angle.y));
}
