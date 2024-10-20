use std::{sync::mpsc, thread::sleep, time::Instant};

use crate::{
    config::{AimbotConfig, DUR},
    message::Message,
};

#[derive(Debug)]
pub struct Deadlock {
    tx: mpsc::Sender<Message>,
    rx: mpsc::Receiver<Message>,
    config: AimbotConfig,
}

impl Deadlock {
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

        let elapsed = start.elapsed();
        if elapsed < DUR {
            sleep(DUR - elapsed);
        }
    }
}
