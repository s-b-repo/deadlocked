use std::{sync::mpsc, thread};

use cs2::Aimbot;
use eframe::egui;
use gui::Gui;

mod bones;
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
mod offsets;
mod process_handle;
mod target;
mod weapon_class;

#[cfg(not(target_os = "linux"))]
compile_error!("only linux is supported.");

fn main() {
    let (tx_cs2, rx_gui) = mpsc::channel();
    let (tx_gui, rx_cs2) = mpsc::channel();

    #[allow(unused)]
    let cs2_thread = match thread::Builder::new()
        .name(String::from("deadlocked_cs2"))
        .spawn(move || {
            Aimbot::new(tx_cs2, rx_cs2).run();
        }) {
        Ok(cs2) => cs2,
        Err(_) => return,
    };

    let default_size = [700.0, 300.0];
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size(default_size)
            .with_min_inner_size(default_size)
            .with_resizable(false),
        ..Default::default()
    };
    eframe::run_native(
        "deadlocked",
        options,
        Box::new(|cc| {
            cc.egui_ctx.set_pixels_per_point(1.5);
            Ok(Box::new(Gui::new(tx_gui, rx_gui)))
        }),
    )
    .unwrap();
}
