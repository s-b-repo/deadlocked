#include "vecmath.h"

#include <math.h>

#define PI 3.14159265358979323846

void vec3_normalize(Vec3 *vector) {
    const f32 length =
        1.0 / sqrtf(vector->x * vector->x + vector->y * vector->y +
                    vector->z * vector->z);

    vector->x *= length;
    vector->y *= length;
    vector->z *= length;
}

void vec2_clamp(Vec2 *vector) {
    if (vector->x > 89.0 && vector->x <= 180.0) {
        vector->x = 89.0;
    }
    if (vector->x > 180.0) {
        vector->x = vector->x - 360.0;
    }
    if (vector->x < -89.0) {
        vector->x = -89.0;
    }
    vector->y = fmodf(vector->y + 180.0, 360.0) - 180.0;
}

void angles_from_vector(const Vec3 *forward, Vec2 *angles) {
    f32 yaw = 0.0;
    f32 pitch = 0.0;

    // forward vector points up or down
    if (forward->x == 0.0 && forward->y == 0.0) {
        pitch = (forward->z > 0.0) ? 270.0 : 90.0;
    } else {
        yaw = atan2f(forward->y, forward->x) * 180.0 / PI;
        if (yaw < 0.0) {
            yaw += 360.0;
        }

        pitch = atan2f(-forward->z, sqrtf(forward->x * forward->x +
                                          forward->y * forward->y)) *
                180.0 / PI;
        if (pitch < 0.0) {
            pitch += 360.0;
        }
    }

    angles->x = pitch;
    angles->y = yaw;
}

f32 angles_to_fov(const Vec2 *view_angles, const Vec2 *aim_angles) {
    Vec2 delta = {.x = view_angles->x - aim_angles->x,
                  .y = view_angles->y - aim_angles->y};

    if (delta.x > 180.0) {
        delta.x = 360.0 - delta.x;
    }
    if (delta.x < 0.0) {
        delta.x = -delta.x;
    }

    // clamp?
    delta.y = fmodf(delta.y + 180.0, 360.0) - 180.0;
    if (delta.y < 0) {
        delta.y = -delta.y;
    }

    return sqrtf(delta.x * delta.x + delta.y * delta.y);
}
