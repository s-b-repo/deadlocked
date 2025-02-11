#include "config.hpp"

toml::array imvec4_to_array(const ImVec4 &vec) {
    toml::array arr;
    arr.push_back(vec.x);
    arr.push_back(vec.y);
    arr.push_back(vec.z);
    arr.push_back(vec.w);
    return arr;
}

ImVec4 array_to_imvec4(const toml::array &arr) {
    ImVec4 vec;
    if (arr.size() >= 4) {
        vec.x = arr[0].value_or(1.0f);
        vec.y = arr[1].value_or(1.0f);
        vec.z = arr[2].value_or(1.0f);
        vec.w = arr[3].value_or(1.0f);
    }
    return vec;
}

toml::table AimbotConfig::to_toml() const {
    return toml::table{
        {"hotkey", static_cast<int>(hotkey)},
        {"start_bullet", start_bullet},
        {"fov", fov},
        {"smooth", smooth},
        {"enabled", enabled},
        {"aim_lock", aim_lock},
        {"visibility_check", visibility_check},
        {"multibone", multibone},
        {"flash_check", flash_check},
        {"rcs", rcs}};
}

AimbotConfig AimbotConfig::from_toml(const toml::table &tbl) {
    AimbotConfig cfg;
    cfg.hotkey = static_cast<KeyCode>(tbl["hotkey"].value_or(static_cast<int>(cfg.hotkey)));
    cfg.start_bullet = tbl["start_bullet"].value_or(cfg.start_bullet);
    cfg.fov = tbl["fov"].value_or(cfg.fov);
    cfg.smooth = tbl["smooth"].value_or(cfg.smooth);
    cfg.enabled = tbl["enabled"].value_or(cfg.enabled);
    cfg.aim_lock = tbl["aim_lock"].value_or(cfg.aim_lock);
    cfg.visibility_check = tbl["visibility_check"].value_or(cfg.visibility_check);
    cfg.multibone = tbl["multibone"].value_or(cfg.multibone);
    cfg.flash_check = tbl["flash_check"].value_or(cfg.flash_check);
    cfg.rcs = tbl["rcs"].value_or(cfg.rcs);
    return cfg;
}

toml::table TriggerbotConfig::to_toml() const {
    return toml::table{
        {"hotkey", static_cast<int>(hotkey)},
        {"delay_min", delay_min},
        {"delay_max", delay_max},
        {"enabled", enabled},
        {"visibility_check", visibility_check},
        {"flash_check", flash_check}};
}

TriggerbotConfig TriggerbotConfig::from_toml(const toml::table &tbl) {
    TriggerbotConfig cfg;
    cfg.hotkey = static_cast<KeyCode>(tbl["hotkey"].value_or(static_cast<int>(cfg.hotkey)));
    cfg.delay_min = tbl["delay_min"].value_or(cfg.delay_min);
    cfg.delay_max = tbl["delay_max"].value_or(cfg.delay_max);
    cfg.enabled = tbl["enabled"].value_or(cfg.enabled);
    cfg.visibility_check = tbl["visibility_check"].value_or(cfg.visibility_check);
    cfg.flash_check = tbl["flash_check"].value_or(cfg.flash_check);
    return cfg;
}

toml::table VisualsConfig::to_toml() const {
    return toml::table{
        {"box_color", imvec4_to_array(box_color)},
        {"skeleton_color", imvec4_to_array(skeleton_color)},
        {"armor_color", imvec4_to_array(armor_color)},
        {"crosshair_color", imvec4_to_array(crosshair_color)},
        {"overlay_fps", overlay_fps},
        {"line_width", line_width},
        {"font_size", font_size},
        {"draw_box", static_cast<int>(draw_box)},
        {"draw_skeleton", static_cast<int>(draw_skeleton)},
        {"enabled", enabled},
        {"draw_health", draw_health},
        {"draw_armor", draw_armor},
        {"draw_name", draw_name},
        {"draw_weapon", draw_weapon},
        {"draw_tags", draw_tags},
        {"dropped_weapons", dropped_weapons},
        {"sniper_crosshair", sniper_crosshair},
        {"dynamic_font", dynamic_font},
        {"debug_window", debug_window}};
}

VisualsConfig VisualsConfig::from_toml(const toml::table &tbl) {
    VisualsConfig cfg;
    if (auto arr = tbl["box_color"].as_array()) cfg.box_color = array_to_imvec4(*arr);
    if (auto arr = tbl["skeleton_color"].as_array()) cfg.skeleton_color = array_to_imvec4(*arr);
    if (auto arr = tbl["armor_color"].as_array()) cfg.armor_color = array_to_imvec4(*arr);
    if (auto arr = tbl["crosshair_color"].as_array()) cfg.crosshair_color = array_to_imvec4(*arr);

    cfg.overlay_fps = tbl["overlay_fps"].value_or(cfg.overlay_fps);
    cfg.line_width = tbl["line_width"].value_or(cfg.line_width);
    cfg.font_size = tbl["font_size"].value_or(cfg.font_size);
    cfg.draw_box = static_cast<DrawStyle>(tbl["draw_box"].value_or(static_cast<int>(cfg.draw_box)));
    cfg.draw_skeleton =
        static_cast<DrawStyle>(tbl["draw_skeleton"].value_or(static_cast<int>(cfg.draw_skeleton)));
    cfg.enabled = tbl["enabled"].value_or(cfg.enabled);
    cfg.draw_health = tbl["draw_health"].value_or(cfg.draw_health);
    cfg.draw_armor = tbl["draw_armor"].value_or(cfg.draw_armor);
    cfg.draw_name = tbl["draw_name"].value_or(cfg.draw_name);
    cfg.draw_weapon = tbl["draw_weapon"].value_or(cfg.draw_weapon);
    cfg.draw_tags = tbl["draw_tags"].value_or(cfg.draw_tags);
    cfg.dropped_weapons = tbl["dropped_weapons"].value_or(cfg.dropped_weapons);
    cfg.sniper_crosshair = tbl["sniper_crosshair"].value_or(cfg.sniper_crosshair);
    cfg.dynamic_font = tbl["dynamic_font"].value_or(cfg.dynamic_font);
    cfg.debug_window = tbl["debug_window"].value_or(cfg.debug_window);
    return cfg;
}

toml::table MiscConfig::to_toml() const {
    return toml::table{
        {"max_flash_alpha", max_flash_alpha},
        {"desired_fov", desired_fov},
        {"no_flash", no_flash},
        {"fov_changer", fov_changer}};
}

MiscConfig MiscConfig::from_toml(const toml::table &tbl) {
    MiscConfig cfg;
    cfg.max_flash_alpha = tbl["max_flash_alpha"].value_or(cfg.max_flash_alpha);
    cfg.desired_fov = tbl["desired_fov"].value_or(cfg.desired_fov);
    cfg.no_flash = tbl["no_flash"].value_or(cfg.no_flash);
    cfg.fov_changer = tbl["fov_changer"].value_or(cfg.fov_changer);
    return cfg;
}

toml::table Config::to_toml() const {
    return toml::table{
        {"aimbot", aimbot.to_toml()},
        {"triggerbot", triggerbot.to_toml()},
        {"visuals", visuals.to_toml()},
        {"misc", misc.to_toml()}};
}

Config Config::from_toml(const toml::table &tbl) {
    Config cfg;
    if (auto table_aimbot = tbl["aimbot"].as_table())
        cfg.aimbot = AimbotConfig::from_toml(*table_aimbot);
    if (auto table_triggerbot = tbl["triggerbot"].as_table())
        cfg.triggerbot = TriggerbotConfig::from_toml(*table_triggerbot);
    if (auto table_visuals = tbl["visuals"].as_table())
        cfg.visuals = VisualsConfig::from_toml(*table_visuals);
    if (auto table_misc = tbl["misc"].as_table()) cfg.misc = MiscConfig::from_toml(*table_misc);
    return cfg;
}
