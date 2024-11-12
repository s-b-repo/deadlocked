use std::{fs::File, sync::mpsc, thread::sleep, time::Instant};

use glam::Vec2;

use crate::{
    config::{Config, SLEEP_DURATION},
    cs2::CS2,
    message::{Game, MouseStatus},
    mouse::mouse_valid,
};

use crate::{
    config::{parse_config, AimbotStatus, LOOP_DURATION},
    message::Message,
    mouse::{move_mouse, open_mouse},
};

pub trait Aimbot: std::fmt::Debug {
    fn is_valid(&self) -> bool;
    fn setup(&mut self);
    fn run(&mut self, config: &Config) -> Option<Vec2>;
}

pub struct AimbotManager {
    tx: mpsc::Sender<Message>,
    rx: mpsc::Receiver<Message>,
    config: Config,
    mouse: File,
    aimbot: Box<dyn Aimbot>,
}

impl AimbotManager {
    pub fn new(tx: mpsc::Sender<Message>, rx: mpsc::Receiver<Message>) -> Self {
        let (mouse, path) = open_mouse().unwrap();
        if path == "/dev/null" {
            tx.send(Message::MouseStatus(MouseStatus::PermissionsRequired))
                .unwrap();
        }
        let config = parse_config();
        let aimbot = Box::new(match config.current_game {
            Game::CS2 => CS2::new(),
            Game::Deadlock => CS2::new(),
        });
        Self {
            tx,
            rx,
            config,
            mouse,
            aimbot,
        }
    }

    fn send_message(&mut self, message: Message) {
        self.tx.send(message).unwrap();
    }

    pub fn run(&mut self) {
        self.send_message(Message::Status(AimbotStatus::GameNotStarted));
        let mut previous_status = AimbotStatus::GameNotStarted;
        loop {
            let start = Instant::now();
            let mut mouse_valid = mouse_valid(&mut self.mouse);
            while let Ok(message) = self.rx.try_recv() {
                self.parse_message(message);
            }

            if !mouse_valid {
                self.send_message(Message::MouseStatus(MouseStatus::Disconnected));
                if let Some((mouse, path)) = open_mouse() {
                    mouse_valid = true;
                    if path == "/dev/null" {
                        self.send_message(Message::MouseStatus(MouseStatus::PermissionsRequired));
                    }
                    self.send_message(Message::MouseStatus(MouseStatus::Working));
                    self.mouse = mouse;
                }
            }

            if !self.aimbot.is_valid() {
                if previous_status == AimbotStatus::Working {
                    self.send_message(Message::Status(AimbotStatus::GameNotStarted));
                    previous_status = AimbotStatus::GameNotStarted;
                }
                self.aimbot.setup();
            }
            if mouse_valid && self.aimbot.is_valid() {
                if previous_status == AimbotStatus::GameNotStarted {
                    self.send_message(Message::Status(AimbotStatus::Working));
                    previous_status = AimbotStatus::Working;
                }
                if let Some(coords) = self.aimbot.run(&self.config) {
                    move_mouse(&mut self.mouse, coords);
                }
            }

            if self.aimbot.is_valid() && mouse_valid {
                let elapsed = start.elapsed();
                if elapsed < LOOP_DURATION {
                    sleep(LOOP_DURATION - elapsed);
                }
                sleep(LOOP_DURATION);
            } else {
                sleep(SLEEP_DURATION);
            }
        }
    }

    fn parse_message(&mut self, message: Message) {
        if let Message::ChangeGame(game) = message {
            self.config.current_game = game;
            return;
        }
        let config = self
            .config
            .games
            .get_mut(&self.config.current_game)
            .unwrap();
        match message {
            Message::ConfigHotkey(hotkey) => config.hotkey = hotkey,
            Message::ConfigStartBullet(start_bullet) => config.start_bullet = start_bullet,
            Message::ConfigAimLock(aim_lock) => config.aim_lock = aim_lock,
            Message::ConfigVisibilityCheck(visibility_check) => {
                config.visibility_check = visibility_check
            }
            Message::ConfigFOV(fov) => config.fov = fov,
            Message::ConfigSmooth(smooth) => config.smooth = smooth,
            Message::ConfigMultibone(multibone) => config.multibone = multibone,
            _ => {}
        }
    }
}
