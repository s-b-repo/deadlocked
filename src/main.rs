use std::{sync::mpsc, thread};

use cs2::CS2;
use deadlock::Deadlock;
use eframe::egui;
use gui::Gui;

mod colors;
mod config;
mod constants;
mod cs2;
mod deadlock;
mod gui;
mod key_codes;
mod memory;
mod message;
mod process_handle;
mod target;
mod weapon_class;

#[cfg(not(target_os = "linux"))]
compile_error!("only linux is supported.");

fn main() {
    let (tx_cs2, rx_gui) = mpsc::channel();
    let (tx_gui_cs2, rx_cs2) = mpsc::channel();
    let (tx_gui_deadlock, rx_deadlock) = mpsc::channel();
    let tx_deadlock = tx_cs2.clone();
    let txs_gui = vec![tx_gui_cs2, tx_gui_deadlock];

    let cs2 = thread::Builder::new()
        .name(String::from("deadlocked_cs2"))
        .spawn(move || {
            CS2::new(tx_cs2, rx_cs2).run();
        });
    if cs2.is_err() {
        return;
    }

    let deadlock = thread::Builder::new()
        .name(String::from("deadlocked_deadlock"))
        .spawn(move || {
            Deadlock::new(tx_deadlock, rx_deadlock).run();
        });
    if deadlock.is_err() {
        return;
    }

    let default_size = [700.0, 375.0];
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size(default_size)
            .with_min_inner_size(default_size),
        ..Default::default()
    };
    eframe::run_native(
        "deadlocked",
        options,
        Box::new(|cc| {
            cc.egui_ctx.set_pixels_per_point(1.5);
            Ok(Box::new(Gui::new(txs_gui, rx_gui)))
        }),
    )
    .unwrap();
}
