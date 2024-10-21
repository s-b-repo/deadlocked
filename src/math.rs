use glam::{Vec2, Vec3};

pub fn angles_from_vector(forward: Vec3) -> Vec2 {
    let mut yaw = 0.0;
    let mut pitch = 0.0;

    // forward vector points up or down
    if forward.x == 0.0 && forward.y == 0.0 {
        pitch = if forward.z > 0.0 { 270.0 } else { 90.0 };
    } else {
        yaw = forward.y.atan2(forward.x).to_degrees();
        if yaw < 0.0 {
            yaw += 360.0;
        }

        pitch = (-forward.z)
            .atan2((forward.x * forward.x + forward.y * forward.y).sqrt())
            .to_degrees();
        if pitch < 0.0 {
            pitch += 360.0;
        }
    }

    Vec2::new(pitch, yaw)
}

pub fn angles_to_fov(view_angles: Vec2, aim_angles: Vec2) -> f32 {
    let mut delta = view_angles - aim_angles;

    delta.x = delta.x.abs().min(360.0 - delta.x.abs());
    delta.y = ((delta.y + 180.0) % 360.0 - 180.0).abs();

    delta.length()
}

pub fn vec2_clamp(vec: &mut Vec2) {
    vec.x = vec.x.clamp(-89.0, 89.0);
    vec.y = (vec.y + 180.0) % 360.0 - 180.0;
}
