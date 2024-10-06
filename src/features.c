#include "features.h"

#include "bones.h"
#include "config.h"
#include "game.h"

typedef struct Target {
    u64 pawn;
    u64 bone_index;
} Target;

static Target target = {0};

void reset(void) {
    target.pawn = 0;
    target.bone_index = 0;
}

void run(ProcessHandle *process, Offsets *offsets) {
    const u64 local_controller = get_local_controller(process, offsets);
    const u64 local_pawn = get_pawn(process, offsets, local_controller);

    if (!local_pawn) {
        reset();
        return;
    }

    const bool aimbot_active = is_button_down(process, offsets, AIMBOT_BUTTON);
    const Vec2 view_angles = get_view_angles(process, offsets, local_pawn);
    const bool ffa = is_ffa(process, offsets);
    const u8 team = get_team(process, offsets, local_pawn);

    // update target when button not held, or when no target was previously
    // found
    f32 best_fov = 360.0;
    Vec2 best_angle = {0};
    if (!aimbot_active || !target.pawn) {
        for (i32 i = 1; i <= 64; i++) {
            const u64 controller = get_client_entity(process, offsets, i);
            if (!controller) {
                continue;
            }

            const u64 pawn = get_pawn(process, offsets, controller);
            if (!pawn) {
                continue;
            }

            if (!ffa && team == get_team(process, offsets, pawn)) {
                continue;
            }

            const Vec3 head_position =
                get_bone_position(process, offsets, pawn, BONE_HEAD);
        }
    }
}
