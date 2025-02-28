#include "math.hpp"

#include "globals.hpp"

f32 ToDegrees(const f32 value) { return value * 180.0f / M_PI; }

glm::vec2 AnglesFromVector(const glm::vec3 &forward) {
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;

    // forward vector points up or down
    if (forward.x == 0.0f && forward.y == 0.0f) {
        yaw = 0.0f;
        pitch = forward.z > 0.0f ? 270.0f : 90.0f;
    } else {
        yaw = ToDegrees(atan2f(forward.y, forward.x));
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }

        pitch = ToDegrees(atan2f(-forward.z, (glm::length(glm::vec2(forward.x, forward.y)))));
        if (pitch < 0.0f) {
            pitch += 360.0f;
        }
    }

    return glm::vec2(pitch, yaw);
}

f32 AnglesToFov(const glm::vec2 &view_angles, const glm::vec2 &aim_angles) {
    glm::vec2 delta = view_angles - aim_angles;

    if (delta.x > 180.0f) {
        delta.x = 360.0f - delta.x;
    }
    delta.x = fabsf(delta.x);

    // clamp?
    delta.y = fabsf(fmodf((delta.y + 180.0f), 360.0f) - 180.0f);

    return glm::length(delta);
}

void Vec2Clamp(glm::vec2 &vec) {
    if (vec.x > 89.0f && vec.x <= 180.0f) {
        vec.x = 89.0f;
    }
    if (vec.x > 180.0f) {
        vec.x -= 360.0f;
    }
    if (vec.x < -89.0f) {
        vec.x = -89.0f;
    }
    vec.y = fmodf((vec.y + 180.0f), 360.0f) - 180.0f;
}

std::optional<glm::vec2> WorldToScreen(const glm::vec3 &position) {
    glm::vec2 screen_position = glm::vec2{
        view_matrix[0].x * position.x + view_matrix[0].y * position.y +
            view_matrix[0].z * position.z + view_matrix[0].w,
        view_matrix[1].x * position.x + view_matrix[1].y * position.y +
            view_matrix[1].z * position.z + view_matrix[1].w};

    const f32 w = view_matrix[3].x * position.x + view_matrix[3].y * position.y +
                  view_matrix[3].z * position.z + view_matrix[3].w;

    if (w < 0.01f) {
        return std::nullopt;
    }

    screen_position.x /= w;
    screen_position.y /= w;

    f32 x = window_size.z * 0.5f;
    f32 y = window_size.w * 0.5f;

    screen_position.x = x + 0.5f * screen_position.x * window_size.z + 0.5f;
    screen_position.y = y - 0.5f * screen_position.y * window_size.w + 0.5f;

    screen_position.x += window_size.x;
    screen_position.y += window_size.y;

    if (screen_position.x < window_size.x || screen_position.x > window_size.x + window_size.z ||
        screen_position.y < window_size.y || screen_position.y > window_size.y + window_size.w) {
        return std::nullopt;
    }

    return screen_position;
}
