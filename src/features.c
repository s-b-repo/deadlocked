#include "features.h"

#include <math.h>

#include "bones.h"
#include "config.h"
#include "game.h"
#include "mouse.h"
#include "vecmath.h"

typedef struct Target {
    u64 pawn;
    u64 bone_index;
    Vec2 angle;
} Target;

static Target target = {0};

void reset(void) {
    target.pawn = 0;
    target.bone_index = 0;
    target.angle.x = 0.0;
    target.angle.y = 0.0;
}

Vec2 get_target_angle(const ProcessHandle *process, const Offsets *offsets,
                      const u64 local_pawn, const Vec3 position,
                      const Vec2 aim_punch) {
    Vec3 eye_position = get_eye_position(process, offsets, local_pawn);

    Vec3 forward = {.x = position.x - eye_position.x,
                    .y = position.y - eye_position.y,
                    .z = position.z - eye_position.z};

    vec3_normalize(&forward);

    Vec2 angles = {0};
    angles_from_vector(&forward, &angles);

    angles.x -= 2.0 * aim_punch.x;
    angles.y -= 2.0 * aim_punch.y;

    vec2_clamp(&angles);

    return angles;
}

void run(const ProcessHandle *process, const Offsets *offsets) {
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
    const Vec2 aim_punch = get_aim_punch(process, offsets, local_pawn);

    // update target when button not held, or when no target was previously
    // found
    f32 best_fov = 360.0;
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

            if (is_dormant(process, offsets, pawn)) {
                continue;
            }

            if (get_health(process, offsets, pawn) <= 0) {
                continue;
            }

            if (get_life_state(process, offsets, pawn)) {
                continue;
            }

            if (!ffa && team == get_team(process, offsets, pawn)) {
                continue;
            }

            const Vec3 head_position =
                get_bone_position(process, offsets, pawn, BONE_HEAD);

            const Vec2 angle = get_target_angle(process, offsets, local_pawn,
                                                head_position, aim_punch);

            const f32 fov = angles_to_fov(&view_angles, &angle);

            if (fov > AIMBOT_FOV) {
                continue;
            }
            if (fov < best_fov) {
                best_fov = fov;

                target.pawn = pawn;
                target.bone_index = BONE_HEAD;
                target.angle = angle;
            }
        }
    }

    if (best_fov > AIMBOT_FOV && !target.pawn) {
        return;
    }

    Vec2 aim_angles = {.x = view_angles.x - target.angle.x,
                       .y = view_angles.y - target.angle.y};

    vec2_clamp(&aim_angles);

    const f32 sensitivity = get_sensitivity(process, offsets) *
                            get_fov_multiplier(process, offsets, local_pawn);

    // x is y, what?
    Vec2 xy = {.x = (aim_angles.y / sensitivity) / 0.022,
               .y = (aim_angles.x / sensitivity) / -0.022};

    Vec2 smooth_angles = {0};

    if (AIMBOT_SMOOTH >= 1.0) {
        if (fabsf(xy.x) > 1.0) {
            if (smooth_angles.x < xy.x) {
                smooth_angles.x =
                    smooth_angles.x + 1.0 + (xy.x / AIMBOT_SMOOTH);
            } else if (smooth_angles.x > xy.x) {
                smooth_angles.x =
                    smooth_angles.x - 1.0 + (xy.x / AIMBOT_SMOOTH);
            } else {
                smooth_angles.x = xy.x;
            }
        } else {
            smooth_angles.x = xy.x;
        }

        if (fabsf(xy.y) > 1.0) {
            if (smooth_angles.y < xy.y) {
                smooth_angles.y =
                    smooth_angles.y + 1.0 + (xy.y / AIMBOT_SMOOTH);
            } else if (smooth_angles.y > xy.y) {
                smooth_angles.y =
                    smooth_angles.y - 1.0 + (xy.y / AIMBOT_SMOOTH);
            } else {
                smooth_angles.y = xy.y;
            }
        } else {
            smooth_angles.y = xy.y;
        }
    } else {
        smooth_angles.x = xy.x;
        smooth_angles.y = xy.y;
    }

    move_mouse((i32)smooth_angles.x, (i32)smooth_angles.y);
}
