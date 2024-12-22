struct Color {
    float r;
    float g;
    float b;
    float a;
};

struct Config {
    Color box_color;
    Color skeleton_color;
    Color armor_color;
    s32 overlay_fps;
    u8 draw_box;
    u8 draw_skeleton;
    bool visuals_enabled;
    bool draw_health;
    bool draw_armor;
    bool draw_weapon;
    bool visibility_check;
    bool debug_window;
};

Config config @ 0x00;