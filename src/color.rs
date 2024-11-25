use eframe::egui::Color32;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, Serialize, Deserialize)]
pub struct Color {
    pub r: u8,
    pub b: u8,
    pub g: u8,
}

impl Color {
    pub fn from_egui_color(color: Color32) -> Self {
        Self {
            r: color.r(),
            g: color.g(),
            b: color.b(),
        }
    }

    #[allow(unused)]
    pub fn from_sdl_color(color: sdl3::pixels::Color) -> Self {
        Self {
            r: color.r,
            g: color.g,
            b: color.b,
        }
    }

    pub fn egui_color(&self) -> Color32 {
        Color32::from_rgb(self.r, self.g, self.b)
    }

    pub fn sdl_color(&self) -> sdl3::pixels::Color {
        sdl3::pixels::Color::RGB(self.r, self.g, self.b)
    }
}
