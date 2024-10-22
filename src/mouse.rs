use std::{
    fs::{read_dir, File, OpenOptions},
    io::{ErrorKind, Write},
    process::exit,
    time::{SystemTime, UNIX_EPOCH},
};

use glam::{IVec2, Vec2};

use crate::config::DEBUG_WITHOUT_MOUSE;

#[derive(Debug, Clone, Copy)]
struct Timeval {
    seconds: u64,
    microseconds: u64,
}

#[derive(Debug, Clone, Copy)]
struct InputEvent {
    time: Timeval,
    event_type: u16,
    code: u16,
    value: i32,
}

impl InputEvent {
    fn bytes(&self) -> Vec<u8> {
        let mut bytes: Vec<u8> = Vec::with_capacity(24);

        bytes.extend(&self.time.seconds.to_le_bytes());
        bytes.extend(&self.time.microseconds.to_le_bytes());

        bytes.extend(&self.event_type.to_le_bytes());
        bytes.extend(&self.code.to_le_bytes());
        bytes.extend(&self.value.to_le_bytes());

        bytes
    }
}

const EV_SYN: u16 = 0x00;
const EV_REL: u16 = 0x02;
const SYN_REPORT: u16 = 0x00;
const AXIS_X: u16 = 0x00;
const AXIS_Y: u16 = 0x01;

pub fn open_mouse() -> Option<File> {
    if DEBUG_WITHOUT_MOUSE {
        return Some(OpenOptions::new().write(true).open("/dev/null").unwrap());
    }
    for file in read_dir("/dev/input/by-id").unwrap() {
        let entry = file.unwrap();
        let name = entry.file_name().into_string().unwrap();
        if !name.ends_with("event-mouse") {
            continue;
        }

        let path = format!("/dev/input/by-id/{}", name);
        let file = OpenOptions::new().write(true).open(path);
        match file {
            Ok(file) => return Some(file),
            Err(error) => {
                if error.kind() == ErrorKind::PermissionDenied {
                    println!("please execute with sudo");
                    exit(1);
                }
            }
        }
    }
    None
}

pub fn move_mouse(mouse: &mut File, coords: Vec2) {
    if DEBUG_WITHOUT_MOUSE {
        println!("moving mouse: {}", coords);
        return;
    }

    let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();
    let time = Timeval {
        seconds: now.as_secs(),
        microseconds: now.subsec_micros() as u64,
    };

    let xy = IVec2::new(coords.x as i32, coords.y as i32);

    let x = InputEvent {
        time,
        event_type: EV_REL,
        code: AXIS_X,
        value: xy.x,
    };

    let y = InputEvent {
        time,
        event_type: EV_REL,
        code: AXIS_Y,
        value: xy.y,
    };

    let syn = InputEvent {
        time,
        event_type: EV_SYN,
        code: SYN_REPORT,
        value: 0,
    };

    let _ = mouse.write(&x.bytes());
    let _ = mouse.write(&syn.bytes());

    let _ = mouse.write(&y.bytes());
    let _ = mouse.write(&syn.bytes());
}
