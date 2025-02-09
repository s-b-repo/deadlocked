#include "math.hpp"

#include <random>

f32 ToDegrees(const f32 value) { return value * 180.0 / M_PI; }

glm::vec2 AimSmooth(const glm::vec2 &aim_coords, f32 smooth) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution range(-0.5f, 0.5f);
    glm::vec2 smooth_coords = aim_coords / smooth;
    glm::vec2 jitter(range(gen) * smooth_coords.x, range(gen) * smooth_coords.y);
    glm::vec2 out = smooth_coords + jitter;
    return glm::clamp(out, 0.0f, 200.0f / smooth);
}

glm::vec2 AnglesFromVector(const glm::vec3 &forward) {
    f32 yaw = 0.0;
    f32 pitch = 0.0;

    // forward vector points up or down
    if (forward.x == 0.0 && forward.y == 0.0) {
        yaw = 0.0;
        pitch = forward.z > 0.0 ? 270.0 : 90.0;
    } else {
        yaw = ToDegrees(atan2f(forward.y, forward.x));
        if (yaw < 0.0) {
            yaw += 360.0;
        }

        pitch = ToDegrees(atan2f(-forward.z, (glm::length(glm::vec2(forward.x, forward.y)))));
        if (pitch < 0.0) {
            pitch += 360.0;
        }
    }

    return glm::vec2(pitch, yaw);
}

f32 AnglesToFov(const glm::vec2 &view_angles, const glm::vec2 &aim_angles) {
    glm::vec2 delta = view_angles - aim_angles;

    if (delta.x > 180.0) {
        delta.x = 360.0 - delta.x;
    }
    delta.x = fabsf(delta.x);

    // clamp?
    delta.y = fabsf(fmodf((delta.y + 180.0), 360.0) - 180.0);

    return glm::length(delta);
}

void Vec2Clamp(glm::vec2 &vec) {
    if (vec.x > 89.0 && vec.x <= 180.0) {
        vec.x = 89.0;
    }
    if (vec.x > 180.0) {
        vec.x -= 360.0;
    }
    if (vec.x < -89.0) {
        vec.x = -89.0;
    }
    vec.y = fmodf((vec.y + 180.0), 360.0) - 180.0;
}

extern glm::mat4 view_matrix;
extern glm::ivec4 window_size;

std::optional<glm::vec2> WorldToScreen(const glm::vec3 &position) {
    auto screen_position = glm::vec2(view_matrix[0].x * position.x + view_matrix[0].y * position.y +
                                         view_matrix[0].z * position.z + view_matrix[0].w,
                                     view_matrix[1].x * position.x + view_matrix[1].y * position.y +
                                         view_matrix[1].z * position.z + view_matrix[1].w);

    const f32 w = view_matrix[3].x * position.x + view_matrix[3].y * position.y + view_matrix[3].z * position.z +
                  view_matrix[3].w;

    if (w < 0.01) {
        return std::nullopt;
    }

    screen_position.x /= w;
    screen_position.y /= w;

    f32 x = window_size.z / 2.0;
    f32 y = window_size.w / 2.0;

    screen_position.x = x + 0.5 * screen_position.x * window_size.z + 0.5;
    screen_position.y = y - 0.5 * screen_position.y * window_size.w + 0.5;

    screen_position.x += window_size.x;
    screen_position.y += window_size.y;

    if (screen_position.x < window_size.x || screen_position.x > window_size.x + window_size.z ||
        screen_position.y < window_size.y || screen_position.y > window_size.y + window_size.w) {
        return std::nullopt;
    }

    return screen_position;
}
