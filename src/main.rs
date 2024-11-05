use std::{sync::mpsc, thread};

use aimbot::AimbotManager;
use eframe::egui;
use gui::Gui;

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
    let (tx_aimbot, rx_gui) = mpsc::channel();
    let (tx_gui, rx_aimbot) = mpsc::channel();

    #[allow(unused)]
    let cs2_thread = match thread::Builder::new()
        .name(String::from("deadlocked_cs2"))
        .spawn(move || {
            AimbotManager::new(tx_aimbot, rx_aimbot).run();
        }) {
        Ok(cs2) => cs2,
        Err(_) => return,
    };

    let default_size = [700.0, 325.0];
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
            cc.egui_ctx.set_pixels_per_point(1.5);
            Ok(Box::new(Gui::new(tx_gui, rx_gui)))
        }),
    )
    .unwrap();
}
