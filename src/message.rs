use serde::{Deserialize, Serialize};
use strum::EnumIter;

use crate::{config::AimbotStatus, key_codes::KeyCode};

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash, EnumIter, Serialize, Deserialize)]
pub enum Game {
    CS2,
    Deadlock,
}

impl Game {
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

#[derive(Clone, Copy, Debug)]
pub enum Message {
    ConfigEnabled(Game, bool),
    ConfigHotkey(Game, KeyCode),
    ConfigStartBullet(Game, i32),
    ConfigAimLock(Game, bool),
    ConfigVisibilityCheck(Game, bool),
    ConfigFOV(Game, f32),
    ConfigSmooth(Game, f32),
    ConfigMultibone(Game, bool),
    Status(Game, AimbotStatus),
}
