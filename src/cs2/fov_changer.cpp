#include "cs2/features.hpp"
#include "cs2/player.hpp"
#include "globals.hpp"

void FovChanger() {
    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    if (config.misc.fov_changer) {
        local_player->SetFov(config.misc.desired_fov);
    } else {
        local_player->SetFov(90);
    }
}
