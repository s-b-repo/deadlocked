use std::{sync::mpsc, thread};

use cs2::CS2;
use deadlock::Deadlock;
use eframe::egui;
use gui::Gui;

mod config;
mod cs2;
mod deadlock;
mod gui;
mod key_codes;
mod message;

#[cfg(not(target_os = "linux"))]
compile_error!("only linux is supported.");

fn main() {
    let (tx_cs2, rx_gui) = mpsc::channel();
    let (tx_gui_cs2, rx_cs2) = mpsc::channel();
    let (tx_gui_deadlock, rx_deadlock) = mpsc::channel();
    let tx_deadlock = tx_cs2.clone();
    let txs_gui = vec![tx_gui_cs2, tx_gui_deadlock];

    if thread::Builder::new()
        .name(String::from("deadlocked_cs2"))
        .spawn(move || {
            let mut cs2 = CS2::new(tx_cs2, rx_cs2);
            cs2.run();
        })
        .is_err()
    {
        return;
    }

    if thread::Builder::new()
        .name(String::from("deadlocked_deadlock"))
        .spawn(move || {
            let mut deadlock = Deadlock::new(tx_deadlock, rx_deadlock);
            deadlock.run();
        })
        .is_err()
    {
        return;
    }

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([600.0, 400.0]),
        ..Default::default()
    };
    eframe::run_native(
        "deadlocked",
        options,
        Box::new(|cc| {
            egui_extras::install_image_loaders(&cc.egui_ctx);
            Ok(Box::new(Gui::new(txs_gui, rx_gui)))
        }),
    )
    .unwrap();
}
