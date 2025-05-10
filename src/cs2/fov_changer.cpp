#include "cs2/features.hpp"
#include "cs2/player.hpp"
#include "globals.hpp"

void FovChanger() {
    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    const i32 target_fov = config.misc.fov_changer ? config.misc.desired_fov : 90;
    local_player->SetFov(target_fov);
}
