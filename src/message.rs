use glam::{IVec4, Mat4, Vec3};
use serde::{Deserialize, Serialize};
use strum::EnumIter;

use crate::{
    color::Color,
    config::{AimbotStatus, VisualsConfig},
    key_codes::KeyCode,
    mouse::MouseStatus,
};

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize, Hash, EnumIter)]
pub enum Game {
    CS2,
    Deadlock,
}

impl Game {
    #[allow(unused)]
    pub fn lower_string(&self) -> &str {
        match self {
            Game::CS2 => "cs2",
            Game::Deadlock => "deadlock",
        }
    }

    pub fn string(&self) -> &str {
        match self {
            Game::CS2 => "CS2",
            Game::Deadlock => "Deadlock",
        }
    }
}

#[derive(Clone, Debug)]
pub enum AimbotMessage {
    ConfigEnableAimbot(bool),
    ConfigHotkey(KeyCode),
    ConfigStartBullet(i32),
    ConfigAimLock(bool),
    ConfigVisibilityCheck(bool),
    ConfigFOV(f32),
    ConfigSmooth(f32),
    ConfigMultibone(bool),
    ConfigEnableRCS(bool),
    Status(AimbotStatus),
    ChangeGame(Game),
    MouseStatus(MouseStatus),
    Quit,
}

#[derive(Clone, Copy, Debug, Default, Serialize, Deserialize, PartialEq, EnumIter)]
pub enum DrawStyle {
    #[default]
    None,
    Color,
    Health,
}

#[derive(Clone, Debug)]
pub enum VisualsMessage {
    PlayerInfo(Option<Vec<PlayerInfo>>),
    ViewMatrix(Mat4),
    WindowSize(IVec4),
    EnableVisuals(bool),
    DrawBox(DrawStyle),
    BoxColor(Color),
    DrawSkeleton(DrawStyle),
    SkeletonColor(Color),
    DrawName(DrawStyle),
    NameColor(Color),
    DrawHealth(bool),
    DrawArmor(bool),
    DrawWeaponName(bool),
    VisibilityCheck(bool),

    VisualsFps(u64),
    Config(VisualsConfig),
    Quit,
}

#[allow(unused)]
#[derive(Clone, Debug, Default)]
pub struct PlayerInfo {
    pub name: String, // todo: add player name rendering
    pub health: i32,
    pub armor: i32,
    pub weapon: String, // todo: add player weapon name rendering
    pub position: Vec3,
    pub head: Vec3,
    pub bones: Vec<(Vec3, Vec3)>,
    pub visible: bool,
}
