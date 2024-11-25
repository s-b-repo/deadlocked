use std::{
    sync::mpsc::Receiver,
    time::{Duration, Instant},
};

use glam::{IVec4, Mat4};
use log::warn;
use sdl3::{
    pixels::Color,
    rect::Point,
    render::{Canvas, FPoint, FRect},
    video::Window,
};

use crate::{
    config::VisualsConfig,
    math::world_to_screen,
    message::{DrawStyle, PlayerInfo, VisualsMessage},
};

#[derive(Debug, Default)]
struct DrawInfo {
    view_matrix: Mat4,
    window_size: IVec4,
}

pub fn visuals(rx_aimbot: Receiver<VisualsMessage>, rx_gui: Receiver<VisualsMessage>) {
    let context = sdl3::init().unwrap();
    let video = context.video().unwrap();

    let mut size = Point::new(0, 0);
    for i in 0..video.num_video_drivers().unwrap() {
        if let Ok(bounds) = video.display_bounds(i as u32) {
            let x = bounds.x + bounds.w;
            if x > size.x {
                size.x = x;
            }

            let y = bounds.y + bounds.h;
            if y > size.y {
                size.y = y;
            }
        }
    }

    let w = video
        .window("deadlocked", 1, 1)
        .borderless()
        .hidden()
        .minimized()
        .position(0, 0)
        .build()
        .unwrap();
    let mut window = unsafe {
        video
            .popup_window(&w, size.x as u32, size.y as u32)
            .set_window_flags(0x00000010) // borderless
            .always_on_top()
            .tooltip()
            .transparent()
            .opengl()
            .build()
            .unwrap()
    };
    window.set_position(
        sdl3::video::WindowPos::Positioned(0),
        sdl3::video::WindowPos::Positioned(-50),
    );

    let mut canvas = window.into_canvas();

    canvas.set_draw_color(Color::RGB(255, 128, 0));
    canvas.clear();
    canvas.present();

    let mut event_pump = context.event_pump().unwrap();
    let mut player_info = None;
    let mut draw_info = DrawInfo::default();
    let mut config = VisualsConfig::default();
    'running: loop {
        let start = Instant::now();
        while let Ok(message) = rx_aimbot.try_recv() {
            match message {
                VisualsMessage::PlayerInfo(info) => player_info = info,
                VisualsMessage::ViewMatrix(matrix) => draw_info.view_matrix = matrix,
                VisualsMessage::WindowSize(size) => draw_info.window_size = size,
                _ => {}
            }
        }

        while let Ok(message) = rx_gui.try_recv() {
            match message {
                VisualsMessage::EnableVisuals(enabled) => config.enabled = enabled,
                VisualsMessage::DrawBox(draw_box) => config.draw_box = draw_box,
                VisualsMessage::BoxColor(color) => config.box_color = color,
                VisualsMessage::DrawSkeleton(draw_skeleton) => config.draw_skeleton = draw_skeleton,
                VisualsMessage::SkeletonColor(color) => config.skeleton_color = color,
                VisualsMessage::DrawName(draw_name) => config.draw_name = draw_name,
                VisualsMessage::NameColor(color) => config.name_color = color,
                VisualsMessage::DrawHealth(draw_health) => config.draw_health = draw_health,
                VisualsMessage::DrawArmor(draw_armor) => config.draw_armor = draw_armor,
                VisualsMessage::DrawWeaponName(draw_weapon) => config.draw_weapon = draw_weapon,
                VisualsMessage::VisibilityCheck(visibility_check) => {
                    config.visibility_check = visibility_check
                }
                VisualsMessage::VisualsFps(fps) => config.fps = fps,
                VisualsMessage::Config(c) => config = c,
                VisualsMessage::Quit => break 'running,

                _ => {}
            }
        }

        canvas.set_draw_color(Color::RGBA(0, 0, 0, 0));
        canvas.clear();
        for _ in event_pump.poll_iter() {}

        if !config.enabled {
            end(&mut canvas, start, Duration::from_millis(30));
            continue;
        }

        canvas.set_draw_color(Color::RGB(0, 0, 255));

        if let Some(player_info) = &player_info {
            for player in player_info {
                if !player.visible && config.visibility_check {
                    continue;
                }

                draw_box(&mut canvas, &config, &draw_info, player);
                draw_skeleton(&mut canvas, &config, &draw_info, player);
            }
        } else {
            end(
                &mut canvas,
                start,
                Duration::from_micros(1_000_000 / config.fps),
            );
            continue;
        }

        end(
            &mut canvas,
            start,
            Duration::from_micros(1_000_000 / config.fps),
        );
    }
}

fn draw_box(
    canvas: &mut Canvas<Window>,
    config: &VisualsConfig,
    draw_info: &DrawInfo,
    player: &PlayerInfo,
) {
    if config.draw_box == DrawStyle::None && !config.draw_health && !config.draw_armor {
        return;
    }

    let color = match config.draw_box {
        DrawStyle::None => return,
        DrawStyle::Color => config.box_color.sdl_color(),
        DrawStyle::Health => Color::RGB(
            (255.0 * (100.0 - player.health as f32) / 100.0) as u8,
            (255.0 * player.health as f32 / 100.0) as u8,
            0,
        ),
    };
    let health_color = Color::RGB(
        (255.0 * (100.0 - player.health as f32) / 100.0) as u8,
        (255.0 * player.health as f32 / 100.0) as u8,
        0,
    );
    canvas.set_draw_color(color);

    let screen_position = FPoint::new(
        draw_info.window_size.x as f32,
        draw_info.window_size.y as f32,
    );

    let position = match world_to_screen(
        draw_info.window_size,
        draw_info.view_matrix,
        player.position,
    ) {
        Some(pos) => FPoint::new(
            pos.x as f32 + screen_position.x,
            pos.y as f32 + screen_position.y,
        ),
        None => return,
    };

    let head_position = match world_to_screen(
        draw_info.window_size,
        draw_info.view_matrix,
        player.position + glam::vec3(0.0, 0.0, 75.0),
    ) {
        Some(pos) => FPoint::new(
            pos.x as f32 + screen_position.x,
            pos.y as f32 + screen_position.y,
        ),
        None => return,
    };

    let height = (head_position.y - position.y).abs();
    let width = height / 2.0;

    let line_width = width / 4.0;
    let line_height = height / 4.0;

    let top_left = FPoint::new(head_position.x - width / 2.0, head_position.y);
    let top_right = FPoint::new(head_position.x + width / 2.0, head_position.y);
    let bottom_left = FPoint::new(position.x - width / 2.0, position.y);
    let bottom_right = FPoint::new(position.x + width / 2.0, position.y);

    let draw_box = config.draw_box != DrawStyle::None;

    if draw_box && is_fpoint_on_screen(top_left, draw_info) {
        canvas
            .draw_line(top_left, FPoint::new(top_left.x + line_width, top_left.y))
            .unwrap();
        canvas
            .draw_line(top_left, FPoint::new(top_left.x, top_left.y + line_height))
            .unwrap();
    }

    if draw_box && is_fpoint_on_screen(top_right, draw_info) {
        canvas
            .draw_line(
                top_right,
                FPoint::new(top_right.x - line_width, top_right.y),
            )
            .unwrap();
        canvas
            .draw_line(
                top_right,
                FPoint::new(top_right.x, top_right.y + line_height),
            )
            .unwrap();
    }

    if draw_box && is_fpoint_on_screen(bottom_left, draw_info) {
        canvas
            .draw_line(
                bottom_left,
                FPoint::new(bottom_left.x + line_width, bottom_left.y),
            )
            .unwrap();
        canvas
            .draw_line(
                bottom_left,
                FPoint::new(bottom_left.x, bottom_left.y - line_height),
            )
            .unwrap();
    }

    if draw_box && is_fpoint_on_screen(bottom_right, draw_info) {
        canvas
            .draw_line(
                bottom_right,
                FPoint::new(bottom_right.x - line_width, bottom_right.y),
            )
            .unwrap();
        canvas
            .draw_line(
                bottom_right,
                FPoint::new(bottom_right.x, bottom_right.y - line_height),
            )
            .unwrap();
    }

    let bar_width = line_width / 4.0;
    if config.draw_health {
        canvas.set_draw_color(health_color);
        let bottom_left = FPoint::new(bottom_left.x - bar_width * 1.5, bottom_left.y);
        let height = height * (player.health as f32 / 100.0);
        canvas
            .fill_rect(FRect::new(bottom_left.x, bottom_left.y, bar_width, -height))
            .unwrap();
    }

    if config.draw_armor && player.armor > 0 {
        canvas.set_draw_color(Color::RGB(0, 0, 255));
        let bottom_left = FPoint::new(bottom_left.x - bar_width * 2.5, bottom_left.y);
        let height = height * (player.armor as f32 / 100.0);
        canvas
            .fill_rect(FRect::new(bottom_left.x, bottom_left.y, bar_width, -height))
            .unwrap();
    }
}

fn draw_skeleton(
    canvas: &mut Canvas<Window>,
    config: &VisualsConfig,
    draw_info: &DrawInfo,
    player: &PlayerInfo,
) {
    let color = match config.draw_skeleton {
        DrawStyle::None => return,
        DrawStyle::Color => config.skeleton_color.sdl_color(),
        DrawStyle::Health => Color::RGB(
            (255.0 * (100.0 - player.health as f32) / 100.0) as u8,
            (255.0 * player.health as f32 / 100.0) as u8,
            0,
        ),
    };
    for connection in &player.bones {
        let bone1 = world_to_screen(draw_info.window_size, draw_info.view_matrix, connection.0);
        let bone2 = world_to_screen(draw_info.window_size, draw_info.view_matrix, connection.1);

        if bone1.is_none() || bone2.is_none() {
            continue;
        }

        let screen_position = Point::new(draw_info.window_size.x, draw_info.window_size.y);
        let bone1 = bone1.unwrap() + screen_position;
        let bone2 = bone2.unwrap() + screen_position;

        if bone1.x == 0 && bone1.y == 0 {
            continue;
        }
        if bone2.x == 0 && bone2.y == 0 {
            continue;
        }

        if !is_point_on_screen(bone1, draw_info) {
            continue;
        }
        if !is_point_on_screen(bone2, draw_info) {
            continue;
        }
        canvas.set_draw_color(color);
        canvas.draw_line(bone1, bone2).unwrap();
    }
}

fn end(canvas: &mut Canvas<Window>, start: Instant, dur: Duration) {
    canvas.present();
    let elapsed = start.elapsed();
    if elapsed < dur {
        std::thread::sleep(dur - elapsed);
    } else {
        warn!("visuals loop took {}ms", elapsed.as_millis());
        std::thread::sleep(dur);
    }
}

fn is_point_on_screen(point: Point, draw_info: &DrawInfo) -> bool {
    if point.x < draw_info.window_size.x
        || point.y < draw_info.window_size.y
        || point.x > draw_info.window_size.x + draw_info.window_size.z
        || point.y > draw_info.window_size.y + draw_info.window_size.w
    {
        return false;
    }
    true
}

fn is_fpoint_on_screen(point: FPoint, draw_info: &DrawInfo) -> bool {
    let window_size = glam::vec4(
        draw_info.window_size.x as f32,
        draw_info.window_size.y as f32,
        draw_info.window_size.z as f32,
        draw_info.window_size.w as f32,
    );
    if point.x < window_size.x
        || point.y < window_size.y
        || point.x > window_size.x + window_size.z
        || point.y > window_size.y + window_size.w
    {
        return false;
    }
    true
}
