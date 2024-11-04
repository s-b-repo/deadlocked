use std::time::Duration;

use config::{Config, File};
use serde::{Deserialize, Serialize};

use crate::key_codes::KeyCode;

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
        }
    }
}

pub fn parse_config() -> AimbotConfig {
    let mut config = AimbotConfig::default();
    if !std::path::Path::new(CONFIG_FILE_NAME).exists() {
        return config;
    }

    let config_file = Config::builder()
        .add_source(File::with_name(CONFIG_FILE_NAME))
        .build()
        .unwrap();

    config.enabled = config_file.get("CS2.enabled").unwrap_or(config.enabled);
    config.hotkey = config_file.get("CS2.hotkey").unwrap_or(config.hotkey);
    config.start_bullet = config_file
        .get("CS2.start_bullet")
        .unwrap_or(config.start_bullet);
    config.aim_lock = config_file.get("CS2.aim_lock").unwrap_or(config.aim_lock);
    config.visibility_check = config_file
        .get("CS2.visibility_check")
        .unwrap_or(config.visibility_check);
    config.fov = config_file.get("CS2.fov").unwrap_or(config.fov);
    config.smooth = config_file.get("CS2.smooth").unwrap_or(config.smooth);
    config.multibone = config_file.get("CS2.multibone").unwrap_or(config.multibone);

    config
}
