use std::{sync::mpsc, thread};

use config::ZOOM;
use eframe::egui::{self, FontData, FontDefinitions, Style};

mod aimbot;
mod colors;
mod config;
mod constants;
mod cs2;
mod gui;
mod key_codes;
mod math;
mod memory;
mod message;
mod mouse;
mod process_handle;
mod weapon_class;

#[cfg(not(target_os = "linux"))]
compile_error!("only linux is supported.");

fn main() {
    // todo: this runs as x11 for now, because wayland decorations for winit are not good
    std::env::remove_var("WAYLAND_DISPLAY");

    let username = std::env::var("USER")
        .or_else(|_| std::env::var("USERNAME"))
        .unwrap_or_default();
    if username == "root" {
        println!("start without sudo, and add your user to the input group.");
        std::process::exit(0);
    }

    let (tx_aimbot, rx_gui) = mpsc::channel();
    let (tx_gui, rx_aimbot) = mpsc::channel();

    #[allow(unused)]
    let aimbot_thread = match thread::Builder::new()
        .name(String::from("deadlocked"))
        .spawn(move || {
            aimbot::AimbotManager::new(tx_aimbot, rx_aimbot).run();
        }) {
        Ok(thread) => thread,
        Err(_) => return,
    };

    let default_size = [460.0 * ZOOM, 220.0 * ZOOM];
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size(default_size)
            .with_resizable(false)
            .with_maximize_button(false),
        ..Default::default()
    };
    eframe::run_native(
        "deadlocked",
        options,
        Box::new(|cc| {
            cc.egui_ctx.set_pixels_per_point(ZOOM);

            let font = include_bytes!("../resources/Nunito.ttf");
            let mut font_definitions = FontDefinitions::default();
            font_definitions
                .font_data
                .insert(String::from("inter"), FontData::from_static(font));

            font_definitions
                .families
                .get_mut(&egui::FontFamily::Proportional)
                .unwrap()
                .insert(0, String::from("inter"));

            cc.egui_ctx.set_fonts(font_definitions);

            cc.egui_ctx
                .style_mut_of(egui::Theme::Dark, no_label_interaction);

            Ok(Box::new(gui::Gui::new(tx_gui, rx_gui)))
        }),
    )
    .unwrap();
}

fn no_label_interaction(style: &mut Style) {
    style.interaction.selectable_labels = false;
}
