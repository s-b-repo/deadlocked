#pragma once

#include <imgui.h>

#include "key_code.hpp"
#include "toml.hpp"
#include "types.hpp"

#define VERSION "v5.0.3"

#define FONT "resources/JetBrainsMono.ttf"

// imvec4 toml helper functions
toml::array imvec4_to_array(const ImVec4 &vec);
ImVec4 array_to_imvec4(const toml::array &arr);

enum DrawStyle : u8 {
    DrawNone,
    DrawColor,
    DrawHealth,
};

struct AimbotConfig {
    KeyCode hotkey = KeyCode::Mouse5;
    i32 start_bullet = 2;
    f32 fov = 2.5f;
    f32 smooth = 5.0f;

    bool enabled = true;
    bool aim_lock = false;
    bool visibility_check = true;
    bool multibone = true;
    bool flash_check = true;
    bool rcs = false;

    toml::table to_toml() const;
    static AimbotConfig from_toml(const toml::table &table);
};

struct TriggerbotConfig {
    KeyCode hotkey = KeyCode::Mouse4;
    i32 delay_min = 100;
    i32 delay_max = 200;

    bool enabled = false;
    bool visibility_check = true;
    bool flash_check = true;
    bool scope_check = true;

    toml::table to_toml() const;
    static TriggerbotConfig from_toml(const toml::table &table);
};

struct VisualsConfig {
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 box_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 skeleton_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 armor_color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    ImVec4 crosshair_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    i32 overlay_fps = 120;
    f32 line_width = 2.0;
    f32 font_size = 16.0;

    DrawStyle draw_box = DrawStyle::DrawColor;
    DrawStyle draw_skeleton = DrawStyle::DrawHealth;
    bool enabled = true;
    bool draw_health = true;
    bool draw_armor = true;
    bool draw_name = true;
    bool draw_weapon = true;
    bool draw_tags = true;
    bool dropped_weapons = true;
    bool sniper_crosshair = true;
    bool dynamic_font = false;
    bool debug_window = false;

    toml::table to_toml() const;
    static VisualsConfig from_toml(const toml::table &table);
};

struct MiscConfig {
    f32 max_flash_alpha = 0.0f;
    i32 desired_fov = 90;

    bool no_flash = false;
    bool fov_changer = false;

    toml::table to_toml() const;
    static MiscConfig from_toml(const toml::table &table);
};

struct Config {
    AimbotConfig aimbot;
    TriggerbotConfig triggerbot;
    VisualsConfig visuals;
    MiscConfig misc;

    toml::table to_toml() const;
    static Config from_toml(const toml::table &table);
};

void SaveConfig();
Config LoadConfig();
void ResetConfig();
