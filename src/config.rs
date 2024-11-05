use std::{collections::HashMap, fs::read_to_string, time::Duration};

use serde::{Deserialize, Serialize};
use strum::IntoEnumIterator;

use crate::{key_codes::KeyCode, message::Game};

pub const DEBUG_WITHOUT_MOUSE: bool = false;

const REFRESH_RATE: u64 = 100;
pub const LOOP_DURATION: Duration = Duration::from_millis(1000 / REFRESH_RATE);
pub const SLEEP_DURATION: Duration = Duration::from_secs(1);
pub const CONFIG_FILE_NAME: &str = "config.toml";

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
pub enum AimbotStatus {
    Working,
    GameNotStarted,
}

impl AimbotStatus {
    pub fn string(&self) -> &str {
        match self {
            AimbotStatus::Working => "Working",
            AimbotStatus::GameNotStarted => "Game Not Started",
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AimbotConfig {
    pub hotkey: KeyCode,
    pub start_bullet: i32,
    pub aim_lock: bool,
    pub visibility_check: bool,
    pub fov: f32,
    pub smooth: f32,
    pub multibone: bool,
}

impl Default for AimbotConfig {
    fn default() -> Self {
        Self {
            hotkey: KeyCode::MouseLeft,
            start_bullet: 2,
            aim_lock: false,
            visibility_check: true,
            fov: 2.5,
            smooth: 5.0,
            multibone: true,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub games: HashMap<Game, AimbotConfig>,
    pub current_game: Game,
}

impl Default for Config {
    fn default() -> Self {
        let mut games = HashMap::new();
        for game in Game::iter() {
            games.insert(game, AimbotConfig::default());
        }
        Self {
            games,
            current_game: Game::CS2,
        }
    }
}

pub fn parse_config() -> Config {
    let mut config = Config::default();
    if !std::path::Path::new(CONFIG_FILE_NAME).exists() {
        return config;
    }

    let config_string = read_to_string(CONFIG_FILE_NAME).unwrap();

    config = toml::from_str(config_string.as_str()).unwrap_or_default();

    config
}
