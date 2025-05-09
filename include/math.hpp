#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <mithril/types.hpp>  // Defines `f32`

/// Converts a forward-facing 3D vector to pitch/yaw angles
glm::vec2 AnglesFromVector(const glm::vec3 &forward);

/// Calculates the field-of-view difference (in degrees) between two sets of angles
f32 AnglesToFov(const glm::vec2 &view_angles, const glm::vec2 &aim_angles);

/// Clamps pitch to [-89, 89] and yaw to [-180, 180] to avoid unnatural angles
void Vec2Clamp(glm::vec2 &vec);

/// Converts a 3D world position to 2D screen coordinates (returns nullopt if not visible)
std::optional<glm::vec2> WorldToScreen(const glm::vec3 &position);
