#include "math.hpp"

#include <mithril/numbers.hpp>
#include "globals.hpp"

f32 ToDegrees(const f32 value) {
    return value * 180.0f / numbers::pi<f32>();
}

glm::vec2 AnglesFromVector(const glm::vec3& forward) {
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;

    if (forward.x == 0.0f && forward.y == 0.0f) {
        // Straight up or down
        yaw = 0.0f;
        pitch = (forward.z > 0.0f) ? 270.0f : 90.0f;
    } else {
        yaw = ToDegrees(std::atan2(forward.y, forward.x));
        if (yaw < 0.0f) yaw += 360.0f;

        const f32 horizontal_length = glm::length(glm::vec2(forward.x, forward.y));
        pitch = ToDegrees(std::atan2(-forward.z, horizontal_length));
        if (pitch < 0.0f) pitch += 360.0f;
    }

    return {pitch, yaw};
}

f32 AnglesToFov(const glm::vec2& view_angles, const glm::vec2& aim_angles) {
    glm::vec2 delta = view_angles - aim_angles;

    delta.x = std::fabs((delta.x > 180.0f) ? (360.0f - delta.x) : delta.x);
    delta.y = std::fabs(std::fmod(delta.y + 180.0f, 360.0f) - 180.0f);

    return glm::length(delta);
}

void Vec2Clamp(glm::vec2& vec) {
    if (vec.x > 89.0f && vec.x <= 180.0f) {
        vec.x = 89.0f;
    } else if (vec.x > 180.0f) {
        vec.x -= 360.0f;
    }

    if (vec.x < -89.0f) {
        vec.x = -89.0f;
    }

    vec.y = std::fmod(vec.y + 180.0f, 360.0f) - 180.0f;
}

std::optional<glm::vec2> WorldToScreen(const glm::vec3& position) {
    glm::vec4 clip_space {
        view_matrix[0].x * position.x + view_matrix[0].y * position.y + view_matrix[0].z * position.z + view_matrix[0].w,
        view_matrix[1].x * position.x + view_matrix[1].y * position.y + view_matrix[1].z * position.z + view_matrix[1].w,
        0.0f, // not used
        view_matrix[3].x * position.x + view_matrix[3].y * position.y + view_matrix[3].z * position.z + view_matrix[3].w
    };

    if (clip_space.w < 0.01f) {
        return std::nullopt;
    }

    glm::vec2 ndc {
        clip_space.x / clip_space.w,
        clip_space.y / clip_space.w
    };

    const glm::vec2 half_screen {window_size.z * 0.5f, window_size.w * 0.5f};

    glm::vec2 screen_pos {
        half_screen.x + ndc.x * half_screen.x + 0.5f + window_size.x,
        half_screen.y - ndc.y * half_screen.y + 0.5f + window_size.y
    };

    const bool in_bounds =
        screen_pos.x >= window_size.x &&
        screen_pos.x <= window_size.x + window_size.z &&
        screen_pos.y >= window_size.y &&
        screen_pos.y <= window_size.y + window_size.w;

    if (!in_bounds) {
        return std::nullopt;
    }

    return screen_pos;
}
