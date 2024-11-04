use eframe::egui::{self, Ui};
use std::sync::mpsc;
use strum::IntoEnumIterator;

use crate::{
    colors::Colors,
    config::{parse_config, AimbotConfig, AimbotStatus, CONFIG_FILE_NAME},
    key_codes::KeyCode,
    message::Message,
};

pub struct Gui {
    tx: mpsc::Sender<Message>,
    rx: mpsc::Receiver<Message>,
    config: AimbotConfig,
    status: AimbotStatus,
}

impl Gui {
    pub fn new(tx: mpsc::Sender<Message>, rx: mpsc::Receiver<Message>) -> Self {
        // read config
        let mut config = parse_config();
        config.enabled = true;
        let status = AimbotStatus::GameNotStarted;
        let out = Self {
            tx,
            rx,
            config,
            status,
        };
        out.write_config();
        out
    }

    fn send_message(&self, message: Message) {
        if let Err(error) = self.tx.send(message) {
            println!("{}", error);
        }

        self.write_config();
    }

    fn add_grid(&mut self, ui: &mut Ui) {
        egui::Grid::new("cs2")
            .num_columns(2)
            .min_col_width(75.0)
            .show(ui, |ui| {
                ui.label("Enabled");
                if ui.checkbox(&mut self.config.enabled, "").changed() {
                    self.send_message(Message::ConfigEnabled(self.config.enabled));
                }
                ui.end_row();

                ui.label("Hotkey")
                    .on_hover_text("which key or mouse button should activate the aimbot");
                egui::ComboBox::new("cs2_hotkey", "")
                    .selected_text(format!("{:?}", self.config.hotkey))
                    .show_ui(ui, |ui| {
                        for key_code in KeyCode::iter() {
                            let text = format!("{:?}", &key_code);
                            if ui
                                .selectable_value(&mut self.config.hotkey, key_code, text)
                                .clicked()
                            {
                                self.send_message(Message::ConfigHotkey(self.config.hotkey));
                            }
                        }
                    });
                ui.end_row();

                ui.label("Start Bullet")
                    .on_hover_text("after how many bullets fired in a row the aimbot should start");
                if ui
                    .add(egui::Slider::new(&mut self.config.start_bullet, 0..=5))
                    .changed()
                {
                    self.send_message(Message::ConfigStartBullet(self.config.start_bullet));
                }
                ui.end_row();

                ui.label("Aim Lock")
                    .on_hover_text("whether the aim should lock onto the target");
                if ui.checkbox(&mut self.config.aim_lock, "").changed() {
                    self.send_message(Message::ConfigAimLock(self.config.aim_lock));
                }
                ui.end_row();

                ui.label("Visibility Check")
                    .on_hover_text("whether to check for player visibility");
                if ui.checkbox(&mut self.config.visibility_check, "").changed() {
                    self.send_message(Message::ConfigVisibilityCheck(self.config.visibility_check));
                }
                ui.end_row();

                ui.label("FOV")
                    .on_hover_text("how much around the crosshair the aimbot should \"see\"");
                if ui
                    .add(egui::Slider::new(&mut self.config.fov, 0.1..=10.0).suffix("°"))
                    .changed()
                {
                    self.send_message(Message::ConfigFOV(self.config.fov));
                }
                ui.end_row();

                ui.label("Smooth")
                    .on_hover_text("how much the aimbot input should be smoothed, higher is more");
                if ui
                    .add(egui::Slider::new(&mut self.config.smooth, 1.0..=10.0))
                    .changed()
                {
                    self.send_message(Message::ConfigSmooth(self.config.smooth));
                }
                ui.end_row();

                ui.label("Multibone").on_hover_text(
                    "whether the aimbot should aim at all of the body, or just the head",
                );
                if ui.checkbox(&mut self.config.multibone, "").changed() {
                    self.send_message(Message::ConfigMultibone(self.config.multibone));
                }
                ui.end_row();
            });
    }

    fn add_game_status(&mut self, ui: &mut Ui) {
        ui.label(
            egui::RichText::new(self.status.string())
                .heading()
                .color(match self.status {
                    AimbotStatus::Working => Colors::GREEN,
                    AimbotStatus::GameNotStarted => Colors::YELLOW,
                    AimbotStatus::Paused => Colors::YELLOW,
                }),
        );
    }

    fn write_config(&self) {
        let out = toml::to_string(&self.config).unwrap();
        std::fs::write(CONFIG_FILE_NAME, out).unwrap();
    }
}

impl eframe::App for Gui {
    fn update(&mut self, ctx: &eframe::egui::Context, _frame: &mut eframe::Frame) {
        if let Ok(Message::Status(status)) = self.rx.try_recv() {
            self.status = status
        }

        egui::CentralPanel::default().show(ctx, |ui| {
            egui::Grid::new("main_grid")
                .num_columns(2)
                .spacing([40.0, 4.0])
                .show(ui, |ui| {
                    self.add_grid(ui);
                    self.add_game_status(ui);
                });
        });
    }
}
