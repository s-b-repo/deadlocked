use std::{sync::mpsc, thread::sleep, time::Instant};

use crate::{
    config::{AimbotConfig, LOOP_DURATION},
    message::{Game, Message},
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
        if let Ok(message) = self.rx.try_recv() {
            self.parse_message(message);
        }

        let elapsed = start.elapsed();
        if elapsed < LOOP_DURATION {
            sleep(LOOP_DURATION - elapsed);
        } else {
            println!("loop exceeded max duration: took {}ms", elapsed.as_millis());
        }
    }

    fn parse_message(&mut self, message: Message) {
        match message {
            Message::ConfigEnabled(Game::Deadlock, enabled) => self.config.enabled = enabled,
            Message::ConfigHotkey(Game::Deadlock, hotkey) => self.config.hotkey = hotkey,
            Message::ConfigStartBullet(Game::Deadlock, start_bullet) => {
                self.config.start_bullet = start_bullet
            }
            Message::ConfigAimLock(Game::Deadlock, aim_lock) => self.config.aim_lock = aim_lock,
            Message::ConfigVisibilityCheck(Game::Deadlock, visibility_check) => {
                self.config.visibility_check = visibility_check
            }
            Message::ConfigFOV(Game::Deadlock, fov) => self.config.fov = fov,
            Message::ConfigSmooth(Game::Deadlock, smooth) => self.config.smooth = smooth,
            Message::ConfigMultibone(Game::Deadlock, multibone) => {
                self.config.multibone = multibone
            }
            Message::ConfigPauseWhenSpectated(Game::Deadlock, pause_when_spectated) => {
                self.config.pause_when_spectated = pause_when_spectated
            }
            _ => {}
        }
    }
}
