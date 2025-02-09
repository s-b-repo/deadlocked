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

struct VisualsConfig {
    ImVec4 box_color;
    ImVec4 skeleton_color;
    ImVec4 armor_color;

    s32 overlay_fps;
    DrawStyle draw_box;
    DrawStyle draw_skeleton;

    bool enabled;
    bool draw_health;
    bool draw_armor;
    bool draw_name;
    bool draw_weapon;
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
    VisualsConfig visuals;
    MiscConfig misc;
};

Config config @ 0x00;
