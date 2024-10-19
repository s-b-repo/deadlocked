use std::{sync::mpsc, thread::sleep, time::Instant};

use crate::{config::{Config, DUR}, message::Message};

#[derive(Debug)]
pub struct CS2 {
    tx: mpsc::Sender<Message>,
    rx: mpsc::Receiver<Message>,
    config: Config,
}

impl CS2 {
    pub fn new(tx: mpsc::Sender<Message>, rx: mpsc::Receiver<Message>) -> Self {
        Self {
            tx,
            rx,
            config: Config::default(),
        }
    }

    pub fn run(&mut self) {

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
