#ifndef VECMATH_H
#define VECMATH_H

#include "types.h"

void vec3_normalize(Vec3 *vector);
void vec2_clamp(Vec2 *vector);
void angles_from_vector(const Vec3 *forward, Vec2 *angles);
f32 angles_to_fov(const Vec2 *view_angles, const Vec2 *aim_angles);

#endif
