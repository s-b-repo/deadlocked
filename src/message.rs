use crate::{config::AimbotStatus, key_codes::KeyCode};

#[derive(Clone, Copy, Debug)]
pub enum Message {
    ConfigEnabled(bool),
    ConfigHotkey(KeyCode),
    ConfigStartBullet(i32),
    ConfigAimLock(bool),
    ConfigVisibilityCheck(bool),
    ConfigFOV(f32),
    ConfigSmooth(f32),
    ConfigMultibone(bool),
    Status(AimbotStatus),
}
