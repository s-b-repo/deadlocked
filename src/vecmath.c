#include "vecmath.h"

#include <math.h>

#define PI 3.14159265358979323846f

void vec3_normalize(Vec3 *vector) {
    const f32 length = 1.0f / sqrtf(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z);

    vector->x *= length;
    vector->y *= length;
    vector->z *= length;
}

void vec2_clamp(Vec2 *vector) {
    if (vector->x > 89.0f && vector->x <= 180.0f) {
        vector->x = 89.0f;
    }
    if (vector->x > 180.0f) {
        vector->x = vector->x - 360.0f;
    }
    if (vector->x < -89.0f) {
        vector->x = -89.0f;
    }
    vector->y = fmodf(vector->y + 180.0f, 360.0f) - 180.0f;
}

void angles_from_vector(const Vec3 *forward, Vec2 *angles) {
    f32 yaw = 0.0;
    f32 pitch = 0.0;

    // forward vector points up or down
    if (forward->x == 0.0f && forward->y == 0.0f) {
        pitch = (forward->z > 0.0f) ? 270.0f : 90.0f;
    } else {
        yaw = atan2f(forward->y, forward->x) * 180.0f / PI;
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }

        pitch = atan2f(-forward->z, sqrtf(forward->x * forward->x + forward->y * forward->y)) * 180.0f / PI;
        if (pitch < 0.0f) {
            pitch += 360.0f;
        }
    }

    angles->x = pitch;
    angles->y = yaw;
}

f32 angles_to_fov(const Vec2 *view_angles, const Vec2 *aim_angles) {
    Vec2 delta = {.x = view_angles->x - aim_angles->x, .y = view_angles->y - aim_angles->y};

    if (delta.x > 180.0f) {
        delta.x = 360.0f - delta.x;
    }
    if (delta.x < 0.0f) {
        delta.x = -delta.x;
    }

    // clamp?
    delta.y = fmodf(delta.y + 180.0f, 360.0f) - 180.0f;
    if (delta.y < 0) {
        delta.y = -delta.y;
    }

    return sqrtf(delta.x * delta.x + delta.y * delta.y);
}
