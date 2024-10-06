#include "vecmath.h"

#include <math.h>

void normalize(Vec3 *vector) {
    const f32 length = 1.0 / sqrtf(vector->x * vector->x + vector->y * vector->y +
                             vector->z * vector->z);

    vector->x *= length;
    vector->y *= length;
    vector->z *= length;
}
