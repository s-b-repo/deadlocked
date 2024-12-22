#pragma once

#include <imgui.h>

#include <mutex>

#include "types.hpp"

#define FONT "resources/JetBrainsMono.ttf"

enum DrawStyle: u8 {
    DrawNone,
    DrawColor,
    DrawHealth,
};

struct Config {
    ImVec4 box_color;
    ImVec4 skeleton_color;
    ImVec4 armor_color;

    i32 overlay_fps;
    DrawStyle draw_box;
    DrawStyle draw_skeleton;

    bool visuals_enabled;
    bool draw_health;
    bool draw_armor;
    bool draw_weapon;
    bool visibility_check;
    bool debug_window;
};

void SaveConfig();
void ResetConfig();
