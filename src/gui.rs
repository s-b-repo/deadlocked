use config::{Config, File};
use eframe::egui::{self, Ui};
use std::{collections::HashMap, sync::mpsc};
use strum::IntoEnumIterator;

use crate::{
    config::{parse_config, AimbotConfig, CONFIG_FILE_NAME},
    key_codes::KeyCode,
    message::{Game, Message},
};

pub struct Gui {
    tx: Vec<mpsc::Sender<Message>>,
    rx: mpsc::Receiver<Message>,
    config: HashMap<Game, AimbotConfig>,
}

impl Gui {
    pub fn new(tx: Vec<mpsc::Sender<Message>>, rx: mpsc::Receiver<Message>) -> Self {
        // read config
        let config = parse_config();
        Self { tx, rx, config }
    }

    fn send_message(&self, message: Message) {
        for tx in &self.tx {
            if let Err(error) = tx.send(message) {
                println!("{}", error);
            }
        }
        self.write_config();
    }

    fn add_game_grid(&mut self, ui: &mut Ui, game: Game) {
        ui.heading(game.upper_string());
        egui::Grid::new(game.string())
            .num_columns(2)
            .min_col_width(75.0)
            .show(ui, |ui| {
                ui.label("Enabled");
                if ui
                    .checkbox(&mut self.config.get_mut(&game).unwrap().enabled, "")
                    .changed()
                {
                    self.send_message(Message::ConfigEnabled(
                        game,
                        self.config.get(&game).unwrap().enabled,
                    ));
                }
                ui.end_row();

                ui.label("Hotkey")
                    .on_hover_text("which key or mouse button should activate the aimbot");
                if egui::ComboBox::new(format!("{}_hotkey", game.string()), "")
                    .selected_text(format!("{:?}", self.config.get(&game).unwrap().hotkey))
                    .show_ui(ui, |ui| {
                        for key_code in KeyCode::iter() {
                            let text = format!("{:?}", &key_code);
                            ui.selectable_value(
                                &mut self.config.get_mut(&game).unwrap().hotkey,
                                key_code,
                                text,
                            );
                        }
                    })
                    .response
                    .changed()
                {
                    self.send_message(Message::ConfigHotkey(
                        game,
                        self.config.get(&game).unwrap().hotkey,
                    ));
                }
                ui.end_row();

                ui.label("Start Bullet")
                    .on_hover_text("after how many bullets fired in a row the aimbot should start");
                if ui
                    .add(egui::Slider::new(
                        &mut self.config.get_mut(&game).unwrap().start_bullet,
                        0..=5,
                    ))
                    .changed()
                {
                    self.send_message(Message::ConfigStartBullet(
                        game,
                        self.config.get(&game).unwrap().start_bullet,
                    ));
                }
                ui.end_row();

                ui.label("Aim Lock");
                if ui
                    .checkbox(&mut self.config.get_mut(&game).unwrap().aim_lock, "")
                    .changed()
                {
                    self.send_message(Message::ConfigAimLock(
                        game,
                        self.config.get(&game).unwrap().aim_lock,
                    ));
                }
                ui.end_row();

                ui.label("Visibility Check");
                if ui
                    .checkbox(
                        &mut self.config.get_mut(&game).unwrap().visibility_check,
                        "",
                    )
                    .changed()
                {
                    self.send_message(Message::ConfigVisibilityCheck(
                        game,
                        self.config.get(&game).unwrap().visibility_check,
                    ));
                }
                ui.end_row();

                ui.label("FOV");
                if ui
                    .add(
                        egui::Slider::new(&mut self.config.get_mut(&game).unwrap().fov, 0.1..=10.0)
                            .suffix("°"),
                    )
                    .changed()
                {
                    self.send_message(Message::ConfigFOV(
                        game,
                        self.config.get(&game).unwrap().fov,
                    ));
                }
                ui.end_row();

                ui.label("Multibone");
                if ui
                    .checkbox(&mut self.config.get_mut(&game).unwrap().multibone, "")
                    .changed()
                {
                    self.send_message(Message::ConfigMultibone(
                        game,
                        self.config.get(&game).unwrap().multibone,
                    ));
                }
                ui.end_row();
            });
    }

    fn write_config(&self) {
        let out = toml::to_string(&self.config).unwrap();
        std::fs::write(CONFIG_FILE_NAME, out).unwrap();
    }
}

impl eframe::App for Gui {
    fn update(&mut self, ctx: &eframe::egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            for game in Game::iter() {
                self.add_game_grid(ui, game);
            }
        });
    }
}
