use eframe::egui::{self, Align2, Color32, Layout, Ui};
use std::{cmp::Ordering, sync::mpsc};
use strum::IntoEnumIterator;

use crate::{
    color::{Color, Colors},
    config::{parse_config, write_config, AimbotStatus, Config, GameConfig},
    key_codes::KeyCode,
    message::{AimbotMessage, DrawStyle, Game, VisualsMessage},
    mouse::MouseStatus,
};

#[derive(Debug, PartialEq)]
enum Tab {
    Aimbot,
    Visuals,
}

pub struct Gui {
    tx_aimbot: mpsc::Sender<AimbotMessage>,
    tx_visuals: mpsc::Sender<VisualsMessage>,
    rx: mpsc::Receiver<AimbotMessage>,
    config: Config,
    status: AimbotStatus,
    mouse_status: MouseStatus,
    current_tab: Tab,
    close_timer: i32,
}

impl Gui {
    pub fn new(
        tx_aimbot: mpsc::Sender<AimbotMessage>,
        tx_visuals: mpsc::Sender<VisualsMessage>,
        rx: mpsc::Receiver<AimbotMessage>,
    ) -> Self {
        // read config
        let config = parse_config();
        let status = AimbotStatus::GameNotStarted;
        tx_visuals
            .send(VisualsMessage::Config(
                config
                    .games
                    .get(&config.current_game)
                    .unwrap()
                    .visuals
                    .clone(),
            ))
            .unwrap();
        let out = Self {
            tx_aimbot,
            tx_visuals,
            rx,
            config,
            status,
            mouse_status: MouseStatus::NoMouseFound,
            current_tab: Tab::Aimbot,
            close_timer: -1,
        };
        write_config(&out.config);
        out
    }

    fn send_message(&self, message: AimbotMessage) {
        let _ = self.tx_aimbot.send(message);
    }

    fn send_visuals_message(&self, message: VisualsMessage) {
        let _ = self.tx_visuals.send(message);
    }

    fn aimbot_grid(&mut self, ui: &mut Ui) {
        let mut game_config = self
            .config
            .games
            .get_mut(&self.config.current_game)
            .unwrap()
            .clone();

        egui::Grid::new("aimbot")
            .num_columns(2)
            .min_col_width(100.0)
            .show(ui, |ui| {
                ui.label("Enable Aimbot")
                    .on_hover_text("general aimbot enable");
                if ui.checkbox(&mut game_config.aimbot.enabled, "").changed() {
                    self.send_message(AimbotMessage::ConfigEnableAimbot(
                        game_config.aimbot.enabled,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Hotkey")
                    .on_hover_text("which key or mouse button should activate the aimbot");
                egui::ComboBox::new("aimbot_hotkey", "")
                    .selected_text(format!("{:?}", game_config.aimbot.hotkey))
                    .show_ui(ui, |ui| {
                        for key_code in KeyCode::iter() {
                            let text = format!("{:?}", &key_code);
                            if ui
                                .selectable_value(&mut game_config.aimbot.hotkey, key_code, text)
                                .clicked()
                            {
                                self.send_message(AimbotMessage::ConfigHotkey(
                                    game_config.aimbot.hotkey,
                                ));
                                self.write_game_config(&game_config);
                            }
                        }
                    });
                ui.end_row();

                ui.label("Start Bullet")
                    .on_hover_text("after how many bullets fired in a row the aimbot should start");
                if ui
                    .add(egui::Slider::new(
                        &mut game_config.aimbot.start_bullet,
                        0..=5,
                    ))
                    .changed()
                {
                    self.send_message(AimbotMessage::ConfigStartBullet(
                        game_config.aimbot.start_bullet,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Aim Lock")
                    .on_hover_text("whether the aim should lock onto the target");
                if ui.checkbox(&mut game_config.aimbot.aim_lock, "").changed() {
                    self.send_message(AimbotMessage::ConfigAimLock(game_config.aimbot.aim_lock));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Visibility Check")
                    .on_hover_text("whether to check for player visibility");
                if ui
                    .checkbox(&mut game_config.aimbot.visibility_check, "")
                    .changed()
                {
                    self.send_message(AimbotMessage::ConfigVisibilityCheck(
                        game_config.aimbot.visibility_check,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("FOV")
                    .on_hover_text("how much around the crosshair the aimbot should \"see\"");
                if ui
                    .add(
                        egui::Slider::new(&mut game_config.aimbot.fov, 0.1..=10.0)
                            .suffix("°")
                            .step_by(0.1),
                    )
                    .changed()
                {
                    self.send_message(AimbotMessage::ConfigFOV(game_config.aimbot.fov));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Smooth")
                    .on_hover_text("how much the aimbot input should be smoothed, higher is more");
                if ui
                    .add(egui::Slider::new(
                        &mut game_config.aimbot.smooth,
                        1.0..=10.0,
                    ))
                    .changed()
                {
                    self.send_message(AimbotMessage::ConfigSmooth(game_config.aimbot.smooth));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Multibone").on_hover_text(
                    "whether the aimbot should aim at all of the body, or just the head",
                );
                if ui.checkbox(&mut game_config.aimbot.multibone, "").changed() {
                    self.send_message(AimbotMessage::ConfigMultibone(game_config.aimbot.multibone));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Enable RCS").on_hover_text(
                    "whether recoil should be compensated when the aimbot is not active",
                );
                if ui.checkbox(&mut game_config.aimbot.rcs, "").changed() {
                    self.send_message(AimbotMessage::ConfigEnableRCS(game_config.aimbot.rcs));
                    self.write_game_config(&game_config);
                }
            });

        *self
            .config
            .games
            .get_mut(&self.config.current_game)
            .unwrap() = game_config.clone();
    }

    fn visuals_grid(&mut self, ui: &mut Ui) {
        let mut game_config = self
            .config
            .games
            .get_mut(&self.config.current_game)
            .unwrap()
            .clone();

        egui::Grid::new("visuals")
            .num_columns(2)
            .min_col_width(100.0)
            .show(ui, |ui| {
                ui.label("Enable Visuals")
                    .on_hover_text("general visuals enable");
                if ui.checkbox(&mut game_config.visuals.enabled, "").changed() {
                    self.send_visuals_message(VisualsMessage::EnableVisuals(
                        game_config.visuals.enabled,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Box")
                    .on_hover_text("whether to draw a box, and if so, in which color");
                egui::ComboBox::new("visuals_draw_box", "")
                    .selected_text(format!("{:?}", game_config.visuals.draw_box))
                    .show_ui(ui, |ui| {
                        for draw_style in DrawStyle::iter() {
                            let text = format!("{:?}", &draw_style);
                            if ui
                                .selectable_value(
                                    &mut game_config.visuals.draw_box,
                                    draw_style,
                                    text,
                                )
                                .clicked()
                            {
                                self.send_visuals_message(VisualsMessage::DrawBox(
                                    game_config.visuals.draw_box,
                                ));
                                self.write_game_config(&game_config);
                            }
                        }
                    });
                ui.end_row();

                ui.label("Box Color").on_hover_text(
                    "what color to draw the player box in\nhealth will go from green to red",
                );
                let mut box_color = game_config.visuals.box_color.egui_color();
                if ui.color_edit_button_srgba(&mut box_color).changed() {
                    game_config.visuals.box_color = Color::from_egui_color(box_color.to_opaque());
                    self.send_visuals_message(VisualsMessage::BoxColor(
                        game_config.visuals.box_color,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Skeleton")
                    .on_hover_text("whether to draw player skeletons, and if so, in which color");
                egui::ComboBox::new("visuals_draw_skeleton", "")
                    .selected_text(format!("{:?}", game_config.visuals.draw_skeleton))
                    .show_ui(ui, |ui| {
                        for draw_style in DrawStyle::iter() {
                            let text = format!("{:?}", &draw_style);
                            if ui
                                .selectable_value(
                                    &mut game_config.visuals.draw_skeleton,
                                    draw_style,
                                    text,
                                )
                                .clicked()
                            {
                                self.send_visuals_message(VisualsMessage::DrawSkeleton(
                                    game_config.visuals.draw_skeleton,
                                ));
                                self.write_game_config(&game_config);
                            }
                        }
                    });
                ui.end_row();

                ui.label("Skeleton Color").on_hover_text(
                    "what color to draw the player skeleton in\nhealth will go from green to red",
                );
                let mut skeleton_color = game_config.visuals.skeleton_color.egui_color();
                if ui.color_edit_button_srgba(&mut skeleton_color).changed() {
                    game_config.visuals.skeleton_color =
                        Color::from_egui_color(skeleton_color.to_opaque());
                    self.send_visuals_message(VisualsMessage::SkeletonColor(
                        game_config.visuals.skeleton_color,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label(egui::RichText::new("Name").strikethrough())
                    .on_hover_text("not implemented yet");
                egui::ComboBox::new("visuals_draw_name", "")
                    .selected_text(format!("{:?}", game_config.visuals.draw_name))
                    .show_ui(ui, |ui| {
                        for draw_style in DrawStyle::iter() {
                            let text = format!("{:?}", &draw_style);
                            if ui
                                .selectable_value(
                                    &mut game_config.visuals.draw_name,
                                    draw_style,
                                    text,
                                )
                                .clicked()
                            {
                                self.send_visuals_message(VisualsMessage::DrawName(
                                    game_config.visuals.draw_name,
                                ));
                                self.write_game_config(&game_config);
                            }
                        }
                    });
                ui.end_row();

                ui.label(egui::RichText::new("Name Color").strikethrough())
                    .on_hover_text("not implemented yet");
                let mut name_color = game_config.visuals.name_color.egui_color();
                if ui.color_edit_button_srgba(&mut name_color).changed() {
                    game_config.visuals.name_color = Color::from_egui_color(name_color.to_opaque());
                    self.send_visuals_message(VisualsMessage::NameColor(
                        game_config.visuals.name_color,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Health Bar")
                    .on_hover_text("whether to draw player health\nalways fades from green to red");
                if ui
                    .checkbox(&mut game_config.visuals.draw_health, "")
                    .changed()
                {
                    self.send_visuals_message(VisualsMessage::DrawHealth(
                        game_config.visuals.draw_health,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Armor Bar")
                    .on_hover_text("whether to draw player armor");
                if ui
                    .checkbox(&mut game_config.visuals.draw_armor, "")
                    .changed()
                {
                    self.send_visuals_message(VisualsMessage::DrawArmor(
                        game_config.visuals.draw_armor,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Armor Bar Color")
                    .on_hover_text("what color to draw the player armor bar in");
                let mut armor_color = game_config.visuals.armor_color.egui_color();
                if ui.color_edit_button_srgba(&mut armor_color).changed() {
                    game_config.visuals.armor_color =
                        Color::from_egui_color(armor_color.to_opaque());
                    self.send_visuals_message(VisualsMessage::ArmorColor(
                        game_config.visuals.armor_color,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label(egui::RichText::new("Weapon Names").strikethrough())
                    .on_hover_text("not implemented yet");
                if ui
                    .checkbox(&mut game_config.visuals.draw_weapon, "")
                    .changed()
                {
                    self.send_visuals_message(VisualsMessage::DrawWeaponName(
                        game_config.visuals.draw_weapon,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Visibility Check")
                    .on_hover_text("whether to draw players only when visible");
                if ui
                    .checkbox(&mut game_config.visuals.visibility_check, "")
                    .changed()
                {
                    self.send_visuals_message(VisualsMessage::VisibilityCheck(
                        game_config.visuals.visibility_check,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();

                ui.label("Show Example")
                    .on_hover_text("whether to draw an example player");
                if ui
                    .checkbox(&mut game_config.visuals.draw_example, "")
                    .changed()
                {
                    self.send_visuals_message(VisualsMessage::DrawExample(
                        game_config.visuals.draw_example,
                    ));
                    self.write_game_config(&game_config);
                }
                ui.end_row();


                ui.label("Overlay FPS")
                    .on_hover_text("what fps the overlay should run at");
                if ui
                    .add(egui::Slider::new(&mut game_config.visuals.fps, 30..=240).step_by(5.0))
                    .changed()
                {
                    self.send_visuals_message(VisualsMessage::VisualsFps(game_config.visuals.fps));
                    self.write_game_config(&game_config);
                }
                ui.end_row();
            });

        *self
            .config
            .games
            .get_mut(&self.config.current_game)
            .unwrap() = game_config.clone();
    }

    fn add_game_status(&mut self, ui: &mut Ui) {
        ui.with_layout(Layout::top_down(egui::Align::Center), |ui| {
            ui.label(egui::RichText::new(self.status.string()).heading().color(
                match self.status {
                    AimbotStatus::Working => Colors::GREEN,
                    AimbotStatus::GameNotStarted => Colors::YELLOW,
                },
            ));

            let mouse_text = match &self.mouse_status {
                MouseStatus::Working(name) => name,
                MouseStatus::PermissionsRequired => {
                    "mouse input only works when user is in input group"
                }
                MouseStatus::Disconnected => "mouse was disconnected",
                MouseStatus::NoMouseFound => "no mouse was found",
            };
            let color = if let MouseStatus::Working(_) = &self.mouse_status {
                Color32::PLACEHOLDER
            } else {
                Colors::YELLOW
            };
            ui.label(egui::RichText::new(mouse_text).color(color));
        });
    }

    fn write_game_config(&self, game_config: &GameConfig) {
        let mut config = self.config.clone();
        *config.games.get_mut(&self.config.current_game).unwrap() = game_config.clone();
        write_config(&config);
    }
}

impl eframe::App for Gui {
    fn update(&mut self, ctx: &eframe::egui::Context, _frame: &mut eframe::Frame) {
        // makes it more inefficient to force draw 60fps, but else the mouse disconnect message does not show up
        // todo: when update is split into tick and show, put message parsing into tick and force update the ui when message are received
        ctx.request_repaint();
        if ctx.input(|i| i.viewport().close_requested()) && self.close_timer == -1 {
            self.send_message(AimbotMessage::Quit);
            self.send_visuals_message(VisualsMessage::Quit);
            ctx.send_viewport_cmd(egui::ViewportCommand::CancelClose);
            self.close_timer = 5;
        }
        match self.close_timer.cmp(&0) {
            Ordering::Greater => self.close_timer -= 1,
            Ordering::Equal => ctx.send_viewport_cmd(egui::ViewportCommand::Close),
            _ => {}
        }

        while let Ok(message) = self.rx.try_recv() {
            match message {
                AimbotMessage::Status(status) => self.status = status,
                AimbotMessage::MouseStatus(status) => self.mouse_status = status,
                _ => {}
            }
        }

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.with_layout(egui::Layout::left_to_right(egui::Align::Min), |ui| {
                egui::ComboBox::new("game", "Current Game")
                    .selected_text(self.config.current_game.string())
                    .show_ui(ui, |ui| {
                        for game in Game::iter() {
                            let text = game.string();
                            if ui
                                .selectable_value(&mut self.config.current_game, game.clone(), text)
                                .clicked()
                            {
                                self.send_message(AimbotMessage::ChangeGame(
                                    self.config.current_game.clone(),
                                ));
                                self.send_visuals_message(VisualsMessage::Config(
                                    self.config
                                        .games
                                        .get(&self.config.current_game)
                                        .unwrap()
                                        .visuals
                                        .clone(),
                                ));
                                write_config(&self.config);
                            }
                        }
                    });

                ui.add_sized([5.0, 20.0], egui::widgets::Separator::default().vertical());

                ui.selectable_value(&mut self.current_tab, Tab::Aimbot, "Aimbot");
                ui.selectable_value(&mut self.current_tab, Tab::Visuals, "Visuals");

                ui.add_sized([5.0, 20.0], egui::widgets::Separator::default().vertical());

                if ui.button("Report Issues").clicked() {
                    ctx.open_url(egui::OpenUrl {
                        url: String::from("https://github.com/avitran0/deadlocked/issues"),
                        new_tab: false,
                    });
                }
            });

            ui.separator();

            egui::Grid::new("main_grid")
                .num_columns(2)
                .spacing([20.0, 5.0])
                .show(ui, |ui| {
                    match self.current_tab {
                        Tab::Aimbot => self.aimbot_grid(ui),
                        Tab::Visuals => self.visuals_grid(ui),
                    }
                    self.add_game_status(ui);
                });
        });

        let version = format!(
            "version: {}",
            option_env!("CARGO_PKG_VERSION").unwrap_or("unknown")
        );
        let font = egui::FontId::proportional(12.0);
        let text_size = ctx.fonts(|fonts| {
            fonts
                .layout_no_wrap(version.clone(), font.clone(), Color32::WHITE)
                .size()
        });

        ctx.layer_painter(egui::LayerId::background()).text(
            Align2::RIGHT_BOTTOM
                .align_size_within_rect(text_size, ctx.screen_rect().shrink(4.0))
                .max,
            Align2::RIGHT_BOTTOM,
            version,
            font,
            Colors::SUBTEXT,
        );
    }
}
