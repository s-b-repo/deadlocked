use std::time::Duration;

pub const DUR: Duration = Duration::from_millis(1);

#[derive(Debug)]
pub struct CS2Config {
    pub fov: f32,
    pub multibone: bool,
}

impl Default for CS2Config {
    fn default() -> Self {
        Self {
            fov: 2.5,
            multibone: true,
        }
    }
}

#[derive(Debug)]
pub struct DeadlockConfig {
    pub fov: f32,
    pub multibone: bool,
}

impl Default for DeadlockConfig {
    fn default() -> Self {
        Self {
            fov: 2.5,
            multibone: true,
        }
    }
}

#[derive(Debug, Default)]
pub struct Config {
    pub cs2: CS2Config,
    pub deadlock: DeadlockConfig,
}
