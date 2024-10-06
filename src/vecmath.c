#include "vecmath.h"

#include <math.h>

#define PI 3.14159265358979323846

void normalize(Vec3 *vector) {
    const f32 length =
        1.0 / sqrtf(vector->x * vector->x + vector->y * vector->y +
                    vector->z * vector->z);

    vector->x *= length;
    vector->y *= length;
    vector->z *= length;
}

void angles_from_vector(Vec3 forward, Vec2 *angles) {
    f32 yaw = 0.0;
    f32 pitch = 0.0;

    // forward vector points up or down
    if (forward.x == 0.0 && forward.y == 0.0) {
        pitch = (forward.z > 0.0) ? 270.0 : 90.0;
    } else {
        yaw = atan2f(forward.y, forward.x) * 180.0 / PI;
        if (yaw < 0.0) {
            yaw += 360.0;
        }

        pitch = atan2f(-forward.z,
                       sqrtf(forward.x * forward.x + forward.y * forward.y)) *
                180.0 / PI;
        if (pitch < 0.0) {
            pitch += 360.0;
        }
    }

    angles->x = pitch;
    angles->y = yaw;
}
