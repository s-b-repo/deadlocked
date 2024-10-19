use std::time::Duration;

pub const DUR: Duration = Duration::from_millis(1);

#[derive(Debug)]
pub struct AimbotConfig {
    pub fov: f32,
    pub multibone: bool,
}

impl Default for AimbotConfig {
    fn default() -> Self {
        Self {
            fov: 2.5,
            multibone: true,
        }
    }
}

#[derive(Debug, Default)]
pub struct Config {
    pub cs2: AimbotConfig,
    pub deadlock: AimbotConfig,
}
