use std::{
    fs::{read_dir, File, OpenOptions},
    io::{ErrorKind, Write},
    os::fd::AsRawFd,
    time::{SystemTime, UNIX_EPOCH},
};

use glam::{IVec2, Vec2};
use libc::{c_uint, ioctl, uinput_user_dev};

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

const UI_SET_EVBIT: u64 = 0x40045564;
const UI_SET_RELBIT: u64 = 0x40045566;
const UI_DEV_CREATE: u64 = 0x5501;
const UI_DEV_DESTROY: u64 = 0x5502;
const BUS_USB: u16 = 0x03;

const EV_SYN: u16 = 0x00;
const EV_REL: u16 = 0x02;
const SYN_REPORT: u16 = 0x00;
const REL_X: u16 = 0x00;
const REL_Y: u16 = 0x01;

pub fn open_mouse() -> Option<File> {
    let mut file = match OpenOptions::new().write(true).open("/dev/uinput") {
        Ok(fd) => fd,
        Err(_) => return None,
    };

    unsafe {
        ioctl(file.as_raw_fd(), UI_SET_EVBIT, EV_REL as c_uint);
        ioctl(file.as_raw_fd(), UI_SET_RELBIT, REL_X as c_uint);
        ioctl(file.as_raw_fd(), UI_SET_RELBIT, REL_Y as c_uint);
    }

    let mut uidev = uinput_user_dev {
        name: [0; 80],
        id: libc::input_id {
            bustype: BUS_USB,
            vendor: 0x046D,
            product: 0xC077,
            version: 1,
        },
        ..unsafe { std::mem::zeroed() }
    };
    let name = b"Logitech, Inc. M105 Optical Mouse\0";
    for (i, &b) in name.iter().enumerate() {
        uidev.name[i] = b as i8;
    }

    if file
        .write_all(unsafe {
            std::slice::from_raw_parts(
                &uidev as *const uinput_user_dev as *const u8,
                std::mem::size_of::<uinput_user_dev>(),
            )
        })
        .is_err()
    {
        return None;
    }

    unsafe {
        ioctl(file.as_raw_fd(), UI_DEV_CREATE);
    }

    Some(file)
}

pub fn move_mouse(mouse: &mut File, coords: Vec2) {
    let coords = IVec2::new(coords.x as i32, coords.y as i32);
    if DEBUG_WITHOUT_MOUSE {
        println!("moving mouse: ({} / {})", coords.x, coords.y);
        return;
    }

    let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();
    let time = Timeval {
        seconds: now.as_secs(),
        microseconds: now.subsec_micros() as u64,
    };

    let x = InputEvent {
        time,
        event_type: EV_REL,
        code: REL_X,
        value: coords.x,
    };

    let y = InputEvent {
        time,
        event_type: EV_REL,
        code: REL_Y,
        value: coords.y,
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

pub fn destroy_mouse(mouse: &mut File) {
    unsafe {
        ioctl(mouse.as_raw_fd(), UI_DEV_DESTROY);
    }
}
