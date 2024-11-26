use std::{io::Write, sync::mpsc, thread};

use config::ZOOM;
use eframe::egui::{self, Color32, FontData, FontDefinitions, Style};
use visuals::visuals;

mod aimbot;
mod color;
mod config;
mod constants;
mod cs2;
mod gui;
mod key_codes;
mod math;
mod message;
mod mouse;
mod proc;
mod process_handle;
mod visuals;
mod weapon_class;

#[cfg(not(target_os = "linux"))]
compile_error!("only linux is supported.");

fn main() {
    env_logger::builder()
        .format(|buf, record| writeln!(buf, "{}: {}", record.level(), record.args()))
        .filter_level(log::LevelFilter::Info)
        .init();

    // this runs as x11 for now, because wayland decorations for winit are not good
    // and don't support disabling the maximize button
    std::env::remove_var("WAYLAND_DISPLAY");

    let username = std::env::var("USER").unwrap_or_default();
    if username == "root" {
        println!("start without sudo, and add your user to the input group.");
        std::process::exit(0);
    }

    let (tx_aimbot_to_gui, rx_gui_from_aimbot) = mpsc::channel();
    let (tx_gui_to_aimbot, rx_aimbot_from_gui) = mpsc::channel();
    let (tx_aimbot_to_visuals, rx_visuals) = mpsc::channel();
    let tx_gui_to_visuals = tx_aimbot_to_visuals.clone();

    thread::Builder::new()
        .name(String::from("deadlocked"))
        .spawn(move || {
            aimbot::AimbotManager::new(tx_aimbot_to_gui, tx_aimbot_to_visuals, rx_aimbot_from_gui)
                .run();
        })
        .unwrap();

    thread::Builder::new()
        .name(String::from("deadlocked"))
        .spawn(move || {
            visuals(rx_visuals);
        })
        .unwrap();

    let default_size = [550.0 * ZOOM, 330.0 * ZOOM];
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

            Ok(Box::new(gui::Gui::new(
                tx_gui_to_aimbot,
                tx_gui_to_visuals,
                rx_gui_from_aimbot,
            )))
        }),
    )
    .unwrap();
}

fn no_label_interaction(style: &mut Style) {
    style.interaction.selectable_labels = false;
    style.visuals.override_text_color = Some(Color32::WHITE);
}
