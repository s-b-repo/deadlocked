use std::{collections::HashMap, time::Duration};

use config::{Config, File};
use serde::{Deserialize, Serialize};
use strum::IntoEnumIterator;

use crate::{key_codes::KeyCode, message::Game};

pub const DEBUG_WITHOUT_MOUSE: bool = false;

const REFRESH_RATE: u64 = 100;
pub const LOOP_DURATION: Duration = Duration::from_millis(1000 / REFRESH_RATE);
pub const SLEEP_DURATION: Duration = Duration::from_secs(5);
pub const CONFIG_FILE_NAME: &str = "config.toml";

#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
pub enum AimbotStatus {
    Working,
    Paused,
    GameNotStarted,
}

impl AimbotStatus {
    pub fn string(&self) -> &str {
        match self {
            AimbotStatus::Working => "Working",
            AimbotStatus::Paused => "Paused",
            AimbotStatus::GameNotStarted => "Game Not Started",
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AimbotConfig {
    pub enabled: bool,
    pub hotkey: KeyCode,
    pub start_bullet: i32,
    pub aim_lock: bool,
    pub visibility_check: bool,
    pub fov: f32,
    pub smooth: f32,
    pub multibone: bool,
    pub pause_when_spectated: bool,
}

impl Default for AimbotConfig {
    fn default() -> Self {
        Self {
            enabled: false,
            hotkey: KeyCode::MouseLeft,
            start_bullet: 1,
            aim_lock: false,
            visibility_check: true,
            fov: 1.2,
            smooth: 5.0,
            multibone: true,
            pause_when_spectated: true,
        }
    }
}

pub fn parse_config() -> HashMap<Game, AimbotConfig> {
    let mut config = HashMap::new();
    if !std::path::Path::new(CONFIG_FILE_NAME).exists() {
        for game in Game::iter() {
            config.insert(game, AimbotConfig::default());
        }
        return config;
    }

    let config_file = Config::builder()
        .add_source(File::with_name(CONFIG_FILE_NAME))
        .build()
        .unwrap();
    for game in Game::iter() {
        let mut cfg = AimbotConfig::default();
        let game_str = game.string();

        cfg.enabled = config_file
            .get(format!("{game_str}.enabled").as_str())
            .unwrap_or(cfg.enabled);
        cfg.hotkey = config_file
            .get(format!("{game_str}.hotkey").as_str())
            .unwrap_or(cfg.hotkey);
        cfg.start_bullet = config_file
            .get(format!("{game_str}.start_bullet").as_str())
            .unwrap_or(cfg.start_bullet);
        cfg.aim_lock = config_file
            .get(format!("{game_str}.aim_lock").as_str())
            .unwrap_or(cfg.aim_lock);
        cfg.visibility_check = config_file
            .get(format!("{game_str}.visibility_check").as_str())
            .unwrap_or(cfg.visibility_check);
        cfg.fov = config_file
            .get(format!("{game_str}.fov").as_str())
            .unwrap_or(cfg.fov);
        cfg.smooth = config_file
            .get(format!("{game_str}.smooth").as_str())
            .unwrap_or(cfg.smooth);
        cfg.multibone = config_file
            .get(format!("{game_str}.multibone").as_str())
            .unwrap_or(cfg.multibone);

        config.insert(game, cfg);
    }

    config
}
