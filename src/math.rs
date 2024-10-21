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
