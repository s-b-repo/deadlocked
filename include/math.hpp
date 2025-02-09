#pragma once

#include <glm/glm.hpp>
#include <optional>

#include "types.hpp"

glm::vec2 AimSmooth(const glm::vec2 &aim_coords, f32 smooth);
glm::vec2 AnglesFromVector(const glm::vec3 &forward);
f32 AnglesToFov(const glm::vec2 &view_angles, const glm::vec2 &aim_angles);
void Vec2Clamp(glm::vec2 &vec);
std::optional<glm::vec2> WorldToScreen(const glm::vec3 &position);
