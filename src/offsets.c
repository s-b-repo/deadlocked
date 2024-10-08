#include "offsets.h"

bool all_offsets_found(Offsets *offsets) {
    return offsets->controller.pawn && offsets->pawn.health && offsets->pawn.team && offsets->pawn.life_state &&
           offsets->pawn.weapon && offsets->pawn.fov_multiplier && offsets->pawn.game_scene_node &&
           offsets->pawn.eye_offset && offsets->pawn.aim_punch_cache && offsets->pawn.shots_fired &&
           offsets->pawn.view_angles && offsets->game_scene_node.dormant && offsets->game_scene_node.origin &&
           offsets->game_scene_node.model_state;
}
