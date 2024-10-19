use eframe::egui;
use std::sync::mpsc;

use crate::{config::Config, message::Message};

pub struct Gui {
    tx: Vec<mpsc::Sender<Message>>,
    rx: mpsc::Receiver<Message>,
    config: Config,
}

impl Gui {
    pub fn new(tx: Vec<mpsc::Sender<Message>>, rx: mpsc::Receiver<Message>) -> Self {
        Self {
            tx,
            rx,
            config: Config::default(),
        }
    }
}

impl eframe::App for Gui {
    fn update(&mut self, ctx: &eframe::egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.checkbox(&mut self.config.cs2.multibone, "Multibone");
        });
    }
}
