use std::{
    sync::mpsc::Receiver,
    time::{Duration, Instant},
};

use femtovg::{renderer::OpenGl, Canvas, Color, LineCap, Paint, Path};
use glam::{vec2, IVec4, Mat4, Vec2};
use log::warn;
use sdl3::{rect::Point, render::FPoint, video::Window};

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

pub fn visuals(rx: Receiver<VisualsMessage>) {
    let context = sdl3::init().unwrap();
    let video = context.video().unwrap();

    let gl_attr = video.gl_attr();
    gl_attr.set_context_profile(sdl3::video::GLProfile::Core);
    gl_attr.set_context_version(3, 3);

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
    window.set_opacity(0.0).unwrap();

    let gl_context = window.gl_create_context().unwrap();
    window.gl_make_current(&gl_context).unwrap();
    video.gl_set_swap_interval(0).unwrap();

    let renderer = unsafe {
        OpenGl::new_from_function(|s| video.gl_get_proc_address(s).unwrap() as *const _).unwrap()
    };
    let mut canvas = Canvas::new(renderer).unwrap();
    let size = glam::uvec2(size.x as u32, size.y as u32);
    canvas.set_size(size.x, size.y, 1.0);
    canvas
        .add_font_mem(include_bytes!("../resources/fonts/NunitoSemiBold.ttf"))
        .unwrap();

    let mut event_pump = context.event_pump().unwrap();
    let mut player_info = vec![];
    let mut draw_info = DrawInfo::default();
    let mut config = VisualsConfig::default();
    let transparent = Color::rgba(0, 0, 0, 0);
    'running: loop {
        let start = Instant::now();
        while let Ok(message) = rx.try_recv() {
            match message {
                VisualsMessage::PlayerInfo(info) => player_info = info,
                VisualsMessage::ViewMatrix(matrix) => draw_info.view_matrix = matrix,
                VisualsMessage::WindowSize(size) => draw_info.window_size = size,

                VisualsMessage::EnableVisuals(enabled) => config.enabled = enabled,
                VisualsMessage::DrawBox(draw_box) => config.draw_box = draw_box,
                VisualsMessage::BoxColor(color) => config.box_color = color,
                VisualsMessage::DrawSkeleton(draw_skeleton) => config.draw_skeleton = draw_skeleton,
                VisualsMessage::SkeletonColor(color) => config.skeleton_color = color,
                VisualsMessage::DrawName(draw_name) => config.draw_name = draw_name,
                VisualsMessage::NameColor(color) => config.name_color = color,
                VisualsMessage::DrawHealth(draw_health) => config.draw_health = draw_health,
                VisualsMessage::DrawArmor(draw_armor) => config.draw_armor = draw_armor,
                VisualsMessage::ArmorColor(color) => config.armor_color = color,
                VisualsMessage::DrawWeaponName(draw_weapon) => config.draw_weapon = draw_weapon,
                VisualsMessage::VisibilityCheck(visibility_check) => {
                    config.visibility_check = visibility_check
                }
                VisualsMessage::DrawExample(example) => config.draw_example = example,
                VisualsMessage::VisualsFps(fps) => config.fps = fps,
                VisualsMessage::Config(c) => config = c,
                VisualsMessage::Quit => break 'running,
            }
        }

        canvas.clear_rect(0, 0, size.x, size.y, transparent);
        for _ in event_pump.poll_iter() {}

        if !config.enabled {
            end(&mut canvas, &window, start, Duration::from_millis(30));
            continue;
        }

        if config.draw_example {
            draw_sample(&mut canvas, &config);
        }
        for player in &player_info {
            if !player.visible && config.visibility_check {
                continue;
            }

            draw_box(&mut canvas, &config, &draw_info, player);
            draw_skeleton(&mut canvas, &config, &draw_info, player);
        }

        end(
            &mut canvas,
            &window,
            start,
            Duration::from_micros(1_000_000 / config.fps),
        );
    }
}

fn draw_box(
    canvas: &mut Canvas<OpenGl>,
    config: &VisualsConfig,
    draw_info: &DrawInfo,
    player: &PlayerInfo,
) {
    if config.draw_box == DrawStyle::None && !config.draw_health && !config.draw_armor {
        return;
    }

    let box_color = match config.draw_box {
        DrawStyle::None => return,
        DrawStyle::Color => config.box_color.femtovg_color(),
        DrawStyle::Health => get_health_color(player.health),
    };

    let screen_position = vec2(
        draw_info.window_size.x as f32,
        draw_info.window_size.y as f32,
    );

    let position = match world_to_screen(
        draw_info.window_size,
        draw_info.view_matrix,
        player.position,
    ) {
        Some(pos) => pos + screen_position,
        None => return,
    };

    let mut head_vec = player.position;
    head_vec.z += (player.head.z - player.position.z).abs() + 8.0;
    let head_position =
        match world_to_screen(draw_info.window_size, draw_info.view_matrix, head_vec) {
            Some(pos) => vec2(position.x, pos.y + screen_position.y),
            None => return,
        };

    let height = (head_position.y - position.y).abs();
    let width = height / 2.0;

    let line_width = width / 4.0;
    let line_height = height / 4.0;

    let top_left = vec2(head_position.x - width / 2.0, head_position.y);
    let top_right = vec2(head_position.x + width / 2.0, head_position.y);
    let bottom_left = vec2(position.x - width / 2.0, position.y);
    let bottom_right = vec2(position.x + width / 2.0, position.y);

    let draw_box = config.draw_box != DrawStyle::None;
    let mut path = Path::new();

    if draw_box && is_on_screen(top_left, draw_info) {
        path.move_to(top_left.x, top_left.y);
        path.line_to(top_left.x + line_width, top_left.y);

        path.move_to(top_left.x, top_left.y);
        path.line_to(top_left.x, top_left.y + line_height);
    }

    if draw_box && is_on_screen(top_right, draw_info) {
        path.move_to(top_right.x, top_right.y);
        path.line_to(top_right.x - line_width, top_right.y);

        path.move_to(top_right.x, top_right.y);
        path.line_to(top_right.x, top_right.y + line_height);
    }

    if draw_box && is_on_screen(bottom_left, draw_info) {
        path.move_to(bottom_left.x, bottom_left.y);
        path.line_to(bottom_left.x + line_width, bottom_left.y);

        path.move_to(bottom_left.x, bottom_left.y);
        path.line_to(bottom_left.x, bottom_left.y - line_height);
    }

    if draw_box && is_on_screen(bottom_right, draw_info) {
        path.move_to(bottom_right.x, bottom_right.y);
        path.line_to(bottom_right.x - line_width, bottom_right.y);

        path.move_to(bottom_right.x, bottom_right.y);
        path.line_to(bottom_right.x, bottom_right.y - line_height);
    }
    canvas.stroke_path(
        &path,
        &Paint::color(box_color)
            .with_anti_alias(true)
            .with_line_cap(LineCap::Round)
            .with_line_width(2.0),
    );

    // health bar
    let bar_width = (line_width / 8.0).clamp(1.0, 3.0);
    if config.draw_health
        && is_on_screen(bottom_left, draw_info)
        && is_on_screen(top_left, draw_info)
    {
        let mut path = Path::new();
        let bottom_left = FPoint::new(bottom_left.x - bar_width as f32 * 2.0, bottom_left.y);
        let bar_height = height * (player.health as f32 / 100.0);

        path.rounded_rect(bottom_left.x, bottom_left.y, bar_width, -bar_height, 2.0);
        canvas.fill_path(
            &path,
            &Paint::color(get_health_color(player.health))
                .with_anti_alias(true)
                .with_line_cap(LineCap::Round),
        );
    }

    // armor bar
    if config.draw_armor
        && player.armor > 0
        && is_on_screen(bottom_left, draw_info)
        && is_on_screen(top_left, draw_info)
    {
        let mut path = Path::new();
        let bottom_left = FPoint::new(bottom_left.x - bar_width as f32 * 4.0, bottom_left.y);
        let bar_height = height * (player.armor as f32 / 100.0);

        path.rounded_rect(bottom_left.x, bottom_left.y, bar_width, -bar_height, 2.0);
        canvas.fill_path(
            &path,
            &Paint::color(config.armor_color.femtovg_color())
                .with_anti_alias(true)
                .with_line_cap(LineCap::Round),
        );
    }

    // player name
    if config.draw_name != DrawStyle::None && is_on_screen(top_left, draw_info) {
        let name_color = match config.draw_name {
            DrawStyle::None => Color::white(),
            DrawStyle::Color => config.name_color.femtovg_color(),
            DrawStyle::Health => get_health_color(player.health),
        };
        canvas
            .stroke_text(
                top_left.x,
                top_left.y - 4.0,
                &player.name,
                &Paint::color(name_color)
                    .with_anti_alias(true)
                    .with_line_cap(LineCap::Round)
                    .with_font_size(height / 12.0),
            )
            .unwrap();
    }

    // weapon name
    if config.draw_weapon && is_on_screen(bottom_left, draw_info) {
        canvas
            .stroke_text(
                bottom_left.x,
                bottom_left.y + 4.0 + height / 12.0,
                &player.weapon,
                &Paint::color(Color::white())
                    .with_anti_alias(true)
                    .with_line_cap(LineCap::Round)
                    .with_font_size(height / 12.0),
            )
            .unwrap();
    }
}

fn draw_skeleton(
    canvas: &mut Canvas<OpenGl>,
    config: &VisualsConfig,
    draw_info: &DrawInfo,
    player: &PlayerInfo,
) {
    let skeleton_color = match config.draw_skeleton {
        DrawStyle::None => return,
        DrawStyle::Color => config.skeleton_color.femtovg_color(),
        DrawStyle::Health => get_health_color(player.health),
    };
    let screen_position = vec2(
        draw_info.window_size.x as f32,
        draw_info.window_size.y as f32,
    );
    let mut path = Path::new();
    for connection in &player.bones {
        let bone1 =
            match world_to_screen(draw_info.window_size, draw_info.view_matrix, connection.0) {
                Some(pos) => pos + screen_position,
                None => continue,
            };
        let bone2 =
            match world_to_screen(draw_info.window_size, draw_info.view_matrix, connection.1) {
                Some(pos) => pos + screen_position,
                None => continue,
            };

        if bone1.x < 1.0 && bone1.y < 1.0 {
            continue;
        }
        if bone2.x < 1.0 && bone2.y < 1.0 {
            continue;
        }

        if is_on_screen(bone1, draw_info) && is_on_screen(bone2, draw_info) {
            path.move_to(bone1.x, bone1.y);
            path.line_to(bone2.x, bone2.y);
        }
    }
    canvas.stroke_path(
        &path,
        &Paint::color(skeleton_color)
            .with_anti_alias(true)
            .with_line_cap(LineCap::Round)
            .with_line_width(2.0),
    );
}

fn draw_sample(canvas: &mut Canvas<OpenGl>, config: &VisualsConfig) {
    let box_color = match config.draw_box {
        DrawStyle::None => Color::white(),
        DrawStyle::Color => config.box_color.femtovg_color(),
        DrawStyle::Health => get_health_color(50),
    };

    let position = vec2(100.0, 300.0);

    let head_position = vec2(100.0, 100.0);

    let height = (head_position.y - position.y).abs();
    let width = height / 2.0;

    let line_width = width / 4.0;
    let line_height = height / 4.0;

    let top_left = vec2(head_position.x - width / 2.0, head_position.y);
    let top_right = vec2(head_position.x + width / 2.0, head_position.y);
    let bottom_left = vec2(position.x - width / 2.0, position.y);
    let bottom_right = vec2(position.x + width / 2.0, position.y);

    let draw_box = config.draw_box != DrawStyle::None;
    let mut path = Path::new();

    if draw_box {
        path.move_to(top_left.x, top_left.y);
        path.line_to(top_left.x + line_width, top_left.y);

        path.move_to(top_left.x, top_left.y);
        path.line_to(top_left.x, top_left.y + line_height);

        path.move_to(top_right.x, top_right.y);
        path.line_to(top_right.x - line_width, top_right.y);

        path.move_to(top_right.x, top_right.y);
        path.line_to(top_right.x, top_right.y + line_height);

        path.move_to(bottom_left.x, bottom_left.y);
        path.line_to(bottom_left.x + line_width, bottom_left.y);

        path.move_to(bottom_left.x, bottom_left.y);
        path.line_to(bottom_left.x, bottom_left.y - line_height);

        path.move_to(bottom_right.x, bottom_right.y);
        path.line_to(bottom_right.x - line_width, bottom_right.y);

        path.move_to(bottom_right.x, bottom_right.y);
        path.line_to(bottom_right.x, bottom_right.y - line_height);
    }
    canvas.stroke_path(
        &path,
        &Paint::color(box_color)
            .with_anti_alias(true)
            .with_line_cap(LineCap::Round)
            .with_line_width(2.0),
    );

    // health bar
    let bar_width = (line_width / 8.0).clamp(1.0, 3.0);
    if config.draw_health {
        let mut path = Path::new();
        let bottom_left = FPoint::new(bottom_left.x - bar_width as f32 * 2.0, bottom_left.y);
        let bar_height = height * (50.0 / 100.0);

        path.rounded_rect(bottom_left.x, bottom_left.y, bar_width, -bar_height, 2.0);
        canvas.fill_path(
            &path,
            &Paint::color(get_health_color(50))
                .with_anti_alias(true)
                .with_line_cap(LineCap::Round),
        );
    }

    // armor bar
    if config.draw_armor {
        let mut path = Path::new();
        let bottom_left = FPoint::new(bottom_left.x - bar_width as f32 * 4.0, bottom_left.y);
        let bar_height = height * (75.0 / 100.0);

        path.rounded_rect(bottom_left.x, bottom_left.y, bar_width, -bar_height, 2.0);
        canvas.fill_path(
            &path,
            &Paint::color(config.armor_color.femtovg_color())
                .with_anti_alias(true)
                .with_line_cap(LineCap::Round),
        );
    }

    // player name
    if config.draw_name != DrawStyle::None {
        let name_color = match config.draw_name {
            DrawStyle::None => Color::white(),
            DrawStyle::Color => config.name_color.femtovg_color(),
            DrawStyle::Health => get_health_color(50),
        };
        canvas
            .fill_text(
                top_left.x,
                top_left.y - 4.0,
                "Bot Carl",
                &Paint::color(name_color)
                    .with_anti_alias(true)
                    .with_line_cap(LineCap::Round)
                    .with_font_size(height / 12.0),
            )
            .unwrap();
    }

    // weapon name
    if config.draw_weapon {
        canvas
            .fill_text(
                bottom_left.x,
                bottom_left.y + 4.0 + height / 12.0,
                "ak47",
                &Paint::color(Color::white())
                    .with_anti_alias(true)
                    .with_line_cap(LineCap::Round)
                    .with_font_size(height / 12.0),
            )
            .unwrap();
    }
}

fn end(canvas: &mut Canvas<OpenGl>, window: &Window, start: Instant, dur: Duration) {
    canvas.flush();
    window.gl_swap_window();
    let elapsed = start.elapsed();
    if elapsed < dur {
        std::thread::sleep(dur - elapsed);
    } else {
        warn!("visuals loop took {}ms", elapsed.as_millis());
        std::thread::sleep(dur);
    }
}

fn is_on_screen(point: Vec2, draw_info: &DrawInfo) -> bool {
    let window_size = draw_info.window_size.as_vec4();
    if point.x < window_size.x
        || point.y < window_size.y
        || point.x > window_size.x + window_size.z
        || point.y > window_size.y + window_size.w
    {
        return false;
    }
    true
}

fn get_health_color(health: i32) -> Color {
    let health = health.clamp(0, 100);

    let (r, g, b) = if health <= 50 {
        let factor = health as f32 / 50.0;
        (255, (255.0 * factor) as u8, 0)
    } else {
        let factor = (health as f32 - 50.0) / 50.0;
        ((255.0 * (1.0 - factor)) as u8, 255, 0)
    };

    Color::rgb(r, g, b)
}
