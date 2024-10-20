use std::{sync::mpsc, thread::sleep, time::Instant};

use crate::{
    config::{AimbotConfig, DUR},
    message::{Game, Message},
};

#[derive(Debug)]
pub struct CS2 {
    tx: mpsc::Sender<Message>,
    rx: mpsc::Receiver<Message>,
    config: AimbotConfig,
}

impl CS2 {
    pub fn new(tx: mpsc::Sender<Message>, rx: mpsc::Receiver<Message>) -> Self {
        Self {
            tx,
            rx,
            config: AimbotConfig::default(),
        }
    }

    pub fn run(&mut self) {
        loop {
            self.main_loop();
        }
    }

    fn main_loop(&mut self) {
        let start = Instant::now();

        // todo: work
        if let Ok(message) = self.rx.try_recv() {
            self.parse_message(message);
        }

        let elapsed = start.elapsed();
        if elapsed < DUR {
            sleep(DUR - elapsed);
        }
    }

    fn parse_message(&mut self, message: Message) {
        match message {
            Message::ConfigEnabled(game, enabled) => {
                if game == Game::CS2 {
                    self.config.enabled = enabled
                }
            }
            Message::ConfigHotkey(game, hotkey) => {
                if game == Game::CS2 {
                    self.config.hotkey = hotkey
                }
            }
            Message::ConfigStartBullet(game, start_bullet) => {
                if game == Game::CS2 {
                    self.config.start_bullet = start_bullet
                }
            }
            Message::ConfigFOV(game, fov) => {
                if game == Game::CS2 {
                    self.config.fov = fov
                }
            }
            _ => {}
        }
    }
}
