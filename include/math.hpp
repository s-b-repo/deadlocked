#pragma once

#include <glm/glm.hpp>
#include <mithril/types.hpp>
#include <optional>

glm::vec2 AnglesFromVector(const glm::vec3 &forward);
f32 AnglesToFov(const glm::vec2 &view_angles, const glm::vec2 &aim_angles);
void Vec2Clamp(glm::vec2 &vec);
std::optional<glm::vec2> WorldToScreen(const glm::vec3 &position);
