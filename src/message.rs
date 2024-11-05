use serde::{Deserialize, Serialize};
use strum::EnumIter;

use crate::{config::AimbotStatus, key_codes::KeyCode};

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize, Hash, EnumIter)]
pub enum Game {
    CS2,
    Deadlock,
}

impl Game {
    #[allow(unused)]
    pub fn string(&self) -> &str {
        match self {
            Game::CS2 => "cs2",
            Game::Deadlock => "deadlock",
        }
    }

    pub fn upper_string(&self) -> &str {
        match self {
            Game::CS2 => "CS2",
            Game::Deadlock => "Deadlock",
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum MouseStatus {
    Working,
    SudoRequired,
}

#[derive(Clone, Copy, Debug)]
pub enum Message {
    ConfigHotkey(KeyCode),
    ConfigStartBullet(i32),
    ConfigAimLock(bool),
    ConfigVisibilityCheck(bool),
    ConfigFOV(f32),
    ConfigSmooth(f32),
    ConfigMultibone(bool),
    Status(AimbotStatus),
    MouseStatus(MouseStatus),
    ChangeGame(Game),
}
