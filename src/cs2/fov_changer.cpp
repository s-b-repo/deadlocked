#include <algorithm>

#include "cs2/cs2.hpp"
#include "cs2/features.hpp"

void FovChanger() {
    std::optional<Player> local_player_opt = Player::LocalPlayer();
    if (!local_player_opt.has_value()) {
        return;
    }
    Player local_player = local_player_opt.value();

    if (config.misc.fov_changer) {
        local_player.SetFov(std::clamp(config.misc.desired_fov, 1, 179));
    } else {
        local_player.SetFov(90);
    }
}
