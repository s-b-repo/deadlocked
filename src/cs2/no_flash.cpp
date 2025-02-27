#include "cs2/features.hpp"
#include "cs2/player.hpp"
#include "globals.hpp"

void NoFlash() {
    const std::optional<Player> local_player = Player::LocalPlayer();
    if (!local_player) {
        return;
    }

    if (config.misc.no_flash) {
        local_player->NoFlash(config.misc.max_flash_alpha);
    } else {
        local_player->NoFlash(255.0f);
    }
}
