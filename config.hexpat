struct ImVec4 {
    float r;
    float g;
    float b;
    float a;
};

enum DrawStyle : u8 {
    DrawNone,
    DrawColor,
    DrawHealth,
};

struct AimbotConfig {
    s32 hotkey;
    s32 start_bullet;
    float fov;
    float smooth;

    bool enabled;
    bool aim_lock;
    bool visibility_check;
    bool multibone;
    bool flash_check;
    bool rcs;

    padding[2];
};

struct TriggerbotConfig {
    KeyCode hotkey;
    i32 delay_min;
    i32 delay_max;

    bool enabled;
    bool visibility_check;
    bool flash_check;

    padding[1];
};

struct VisualsConfig {
    ImVec4 box_color;
    ImVec4 skeleton_color;
    ImVec4 armor_color;
    ImVec4 crosshair_color;

    s32 overlay_fps;
    float line_width;

    DrawStyle draw_box;
    DrawStyle draw_skeleton;
    bool enabled;
    bool draw_health;
    bool draw_armor;
    bool draw_name;
    bool draw_weapon;
    bool draw_tags;
    bool dropped_weapons;
    bool sniper_crosshair;
    bool debug_window;

    padding[1];
};

struct MiscConfig {
    float max_flash_alpha;
    s32 desired_fov;

    bool no_flash;
    bool fov_changer;
};

struct Config {
    AimbotConfig aimbot;
    TriggerbotConfig triggerbot;
    VisualsConfig visuals;
    MiscConfig misc;
};

Config config @ 0x00;
