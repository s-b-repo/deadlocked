#include "cs2/cs2.hpp"

#include <string>
#include <thread>

#include "config.hpp"
#include "cs2/constants.hpp"
#include "cs2/features.hpp"
#include "cs2/player.hpp"
#include "log.hpp"
#include "math.hpp"
#include "process.hpp"

bool is_valid = false;
Process process = {0};
Offsets offsets = {0};
Target target;
std::vector<Player> players;

void CS2() {
    Log(LogLevel::Info, "game thread started");
    while (true) {
        const auto clock = std::chrono::steady_clock::now();

        if (flags.should_quit) {
            break;
        }

        if (IsValid()) {
            config_lock.lock();
            Run();
            misc_info.in_game = true;
            config_lock.unlock();
        } else {
            Setup();
            misc_info.in_game = false;
        }

        const auto end_time = std::chrono::steady_clock::now();
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - clock);
        const auto frame_time = std::chrono::microseconds(10000);
        if (IsValid()) {
            std::this_thread::sleep_for(frame_time - us);
        } else {
            // if it was just a 5 second sleep, it would wait 5 seconds before closing the gui
            // window
            for (i32 i = 0; i < 100; i++) {
                config_lock.lock();
                if (flags.should_quit) {
                    return;
                }
                config_lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
}

bool IsValid() {
    if (!process.pid) {
        return false;
    }
    return is_valid && ValidatePid(process.pid);
}

void Setup() {
    const std::optional<i32> pid = GetPid(PROCESS_NAME);
    if (!pid) {
        is_valid = false;
        return;
    }

    const std::optional<Process> new_process = OpenProcess(*pid);
    if (!new_process) {
        is_valid = false;
        return;
    }
    process = *new_process;
    Log(LogLevel::Info, "game started, pid: " + std::to_string(*pid));

    const std::optional<Offsets> new_offsets = FindOffsets();
    if (!new_offsets) {
        is_valid = false;
        return;
    }
    offsets = *new_offsets;

    is_valid = true;
}

std::optional<Offsets> FindOffsets() {
    Offsets offsets = {};

    // get library base addresses, will fail if game is not yet fully loaded
    const auto client_address = process.GetModuleBaseAddress(CLIENT_LIB);
    if (!client_address) {
        return std::nullopt;
    }

    const auto engine_address = process.GetModuleBaseAddress(ENGINE_LIB);
    if (!engine_address) {
        return std::nullopt;
    }

    const auto tier0_address = process.GetModuleBaseAddress(TIER0_LIB);
    if (!tier0_address) {
        return std::nullopt;
    }

    const auto input_address = process.GetModuleBaseAddress(INPUT_LIB);
    if (!input_address) {
        return std::nullopt;
    }

    const auto sdl_address = process.GetModuleBaseAddress(SDL_LIB);
    if (!sdl_address) {
        return std::nullopt;
    }

    const auto matchmaking_address = process.GetModuleBaseAddress(MATCH_LIB);
    if (!matchmaking_address) {
        return std::nullopt;
    }

    offsets.library.client = *client_address;
    offsets.library.engine = *engine_address;
    offsets.library.tier0 = *tier0_address;
    offsets.library.input = *input_address;
    offsets.library.sdl = *sdl_address;
    offsets.library.matchmaking = *matchmaking_address;

    // used for player interface offset
    const auto resource_offset =
        process.GetInterfaceOffset(offsets.library.engine, "GameResourceServiceClientV0");
    if (!resource_offset) {
        Log(LogLevel::Error, "failed to get resource offset");
        return std::nullopt;
    }
    offsets.interface.resource = *resource_offset;
    offsets.interface.entity = process.Read<u64>(offsets.interface.resource + 0x50);
    offsets.interface.player = offsets.interface.entity + 0x10;

    const auto local_player = process.ScanPattern(
        {0x48, 0x83, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0xC3},
        {true, true, true, false, false, false, false, true, true, true, true, true}, 12,
        offsets.library.client);
    if (!local_player) {
        Log(LogLevel::Error, "failed to get local player offset");
        return std::nullopt;
    }
    offsets.direct.local_player = process.GetRelativeAddress(*local_player, 0x03, 0x08);

    const auto cvar_address = process.GetInterfaceOffset(offsets.library.tier0, "VEngineCvar0");
    if (!cvar_address) {
        Log(LogLevel::Error, "failed to get cvar offset");
        return std::nullopt;
    }
    offsets.interface.cvar = *cvar_address;

    const auto input_system_address =
        process.GetInterfaceOffset(offsets.library.input, "InputSystemVersion0");
    if (!input_system_address) {
        Log(LogLevel::Error, "failed to get input offset");
        return std::nullopt;
    }
    offsets.interface.input = *input_system_address;

    const auto view_matrix = process.ScanPattern(
        {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48,
         0x8D, 0x0D},
        {
            true,
            true,
            true,
            false,
            false,
            false,
            false,
            true,
            true,
            true,
            false,
            false,
            false,
            false,
            true,
            true,
            true,
        },
        17, offsets.library.client);
    if (!view_matrix) {
        Log(LogLevel::Error, "could not find view matrix offset");
        return std::nullopt;
    }
    offsets.direct.view_matrix = process.GetRelativeAddress(*view_matrix + 0x07, 0x03, 0x07);

    offsets.direct.button_state =
        process.Read<u32>(process.GetInterfaceFunction(offsets.interface.input, 19) + 0x14);

    const auto game_types = process.ScanPattern(
        {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x48, 0x8B, 0x07},
        {
            true, true,  true,  false, false, false, false, true, true, true,
            true, false, false, false, false, false, true,  true, true,
        },
        19, offsets.library.matchmaking);
    if (!game_types) {
        Log(LogLevel::Error, "could not find map name offset");
        return std::nullopt;
    }
    offsets.direct.game_types = process.GetRelativeAddress(*game_types, 0x03, 0x07);

    const auto sdl_window_address =
        process.GetModuleExport(offsets.library.sdl, "SDL_GetKeyboardFocus");
    if (!sdl_window_address) {
        Log(LogLevel::Error, "could not find sdl window offset");
    }
    const u64 sdl_window = process.GetRelativeAddress(*sdl_window_address, 0x02, 0x06);
    const u64 sdl_window2 = process.Read<u64>(sdl_window);
    offsets.direct.sdl_window = process.GetRelativeAddress(sdl_window2, 0x03, 0x07);

    const auto ffa_address = process.GetConvar(offsets.interface.cvar, "mp_teammates_are_enemies");
    if (!ffa_address) {
        Log(LogLevel::Error, "could not get mp_tammates_are_enemies convar offset");
    }
    offsets.convar.ffa = *ffa_address;
    const auto sensitivity_address = process.GetConvar(offsets.interface.cvar, "sensitivity");
    if (!sensitivity_address) {
        Log(LogLevel::Error, "could not get sensitivity convar offset");
    }
    offsets.convar.sensitivity = *sensitivity_address;

    // dump all netvars from client lib
    const u64 base = offsets.library.client;
    const u64 size = process.ModuleSize(base);

    const std::vector<u8> client_dump_vec = process.DumpModule(base);
    const u8 *client_dump = client_dump_vec.data();

    for (size_t i = (size - 8); i > 0; i -= 8) {
        // read client dump at i from dump directly
        const auto entry = ((u64)client_dump + i);

        bool network_enable = false;
        auto network_enable_name_pointer = *(u64 *)entry;

        if (network_enable_name_pointer == 0) {
            continue;
        }

        if (network_enable_name_pointer >= base && network_enable_name_pointer <= base + size) {
            network_enable_name_pointer =
                *(u64 *)(network_enable_name_pointer - base + client_dump);
            if (network_enable_name_pointer >= base && network_enable_name_pointer <= base + size) {
                const auto name = (char *)(network_enable_name_pointer - base + client_dump);
                if (!strcmp(name, "MNetworkEnable")) {
                    network_enable = true;
                }
            }
        }

        u64 name_pointer = 0;
        if (network_enable == false) {
            name_pointer = *(u64 *)(entry);
        } else {
            name_pointer = *(u64 *)(entry + 0x08);
        }

        if (name_pointer < base || name_pointer > (base + size)) {
            continue;
        }

        const auto name = std::string((char *)(name_pointer - base + client_dump));

        if (name == std::string("m_sSanitizedPlayerName")) {
            if (!network_enable || offsets.controller.name != 0) {
                continue;
            }
            offsets.controller.name = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_hPawn")) {
            if (!network_enable || offsets.controller.pawn != 0) {
                continue;
            }
            offsets.controller.pawn = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_steamID")) {
            if (!network_enable || offsets.controller.steam_id != 0) {
                continue;
            }
            offsets.controller.steam_id = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_iDesiredFOV")) {
            if (offsets.controller.desired_fov != 0) {
                continue;
            }
            offsets.controller.desired_fov = *(i32 *)(entry + 0x08);

        } else if (name == std::string("m_hOwnerEntity")) {
            if (!network_enable || offsets.controller.owner_entity != 0) {
                continue;
            }
            offsets.controller.owner_entity = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_iHealth")) {
            if (!network_enable || offsets.pawn.health != 0) {
                continue;
            }
            offsets.pawn.health = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_ArmorValue")) {
            if (!network_enable || offsets.pawn.armor != 0) {
                continue;
            }
            offsets.pawn.armor = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_iTeamNum")) {
            if (!network_enable || offsets.pawn.team != 0) {
                continue;
            }
            offsets.pawn.team = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_lifeState")) {
            if (!network_enable || offsets.pawn.life_state != 0) {
                continue;
            }
            offsets.pawn.life_state = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_pClippingWeapon")) {
            if (offsets.pawn.weapon != 0) {
                continue;
            }
            offsets.pawn.weapon = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_flFOVSensitivityAdjust")) {
            if (offsets.pawn.fov_multiplier != 0) {
                continue;
            }
            offsets.pawn.fov_multiplier = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_pGameSceneNode")) {
            if (offsets.pawn.game_scene_node != 0) {
                continue;
            }
            offsets.pawn.game_scene_node = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_vecViewOffset")) {
            if (!network_enable || offsets.pawn.eye_offset != 0) {
                continue;
            }
            offsets.pawn.eye_offset = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_aimPunchCache")) {
            if (!network_enable || offsets.pawn.aim_punch_cache != 0) {
                continue;
            }
            offsets.pawn.aim_punch_cache = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_iShotsFired")) {
            if (!network_enable || offsets.pawn.shots_fired != 0) {
                continue;
            }
            offsets.pawn.shots_fired = *(i32 *)(entry + 0x18);
        } else if (name == std::string("v_angle")) {
            if (offsets.pawn.view_angles != 0) {
                continue;
            }
            offsets.pawn.view_angles = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_flFlashMaxAlpha")) {
            if (offsets.pawn.flash_alpha != 0) {
                continue;
            }
            offsets.pawn.flash_alpha = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_flFlashDuration")) {
            if (offsets.pawn.flash_duration != 0) {
                continue;
            }
            offsets.pawn.flash_duration = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_bIsScoped")) {
            if (!network_enable || offsets.pawn.scoped != 0) {
                continue;
            }
            offsets.pawn.scoped = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_entitySpottedState")) {
            if (!network_enable || offsets.pawn.spotted_state != 0) {
                continue;
            }
            const u64 offset = *(i32 *)(entry + 0x18);
            if (offset < 10000 || offset > 14000) {
                continue;
            }
            offsets.pawn.spotted_state = offset;
        } else if (name == std::string("m_iIDEntIndex")) {
            if (offsets.pawn.crosshair_entity != 0) {
                continue;
            }
            offsets.pawn.crosshair_entity = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_pObserverServices")) {
            if (offsets.pawn.observer_services != 0) {
                continue;
            }
            offsets.pawn.observer_services = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_pCameraServices")) {
            if (!network_enable || offsets.pawn.camera_services != 0) {
                continue;
            }
            offsets.pawn.camera_services = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_pItemServices")) {
            if (offsets.pawn.item_services != 0) {
                continue;
            }
            offsets.pawn.item_services = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_pWeaponServices")) {
            if (offsets.pawn.weapon_services != 0) {
                continue;
            }
            offsets.pawn.weapon_services = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_bDormant")) {
            if (offsets.game_scene_node.dormant != 0) {
                continue;
            }
            offsets.game_scene_node.dormant = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_vecAbsOrigin")) {
            if (!network_enable || offsets.game_scene_node.origin != 0) {
                continue;
            }
            offsets.game_scene_node.origin = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_modelState")) {
            if (offsets.game_scene_node.model_state != 0) {
                continue;
            }
            offsets.game_scene_node.model_state = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_bSpotted")) {
            if (offsets.spotted_state.spotted != 0) {
                continue;
            }
            offsets.spotted_state.spotted = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_bSpottedByMask")) {
            if (!network_enable || offsets.spotted_state.mask != 0) {
                continue;
            }
            offsets.spotted_state.mask = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_hObserverTarget")) {
            if (offsets.observer_service.target != 0) {
                continue;
            }
            offsets.observer_service.target = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_iFOV")) {
            if (offsets.camera_service.fov != 0) {
                continue;
            }
            offsets.camera_service.fov = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_bHasDefuser")) {
            if (offsets.item_service.has_defuser != 0) {
                continue;
            }
            offsets.item_service.has_defuser = *(i32 *)(entry + 0x10);
        } else if (name == std::string("m_bHasHelmet")) {
            if (!network_enable || offsets.item_service.has_helmet != 0) {
                continue;
            }
            offsets.item_service.has_helmet = *(i32 *)(entry + 0x18);
        } else if (name == std::string("m_hMyWeapons")) {
            if (offsets.weapon_service.weapons != 0) {
                continue;
            }
            offsets.weapon_service.weapons = *(i32 *)(entry + 0x08);
        }

        if (offsets.AllFound()) {
            Log(LogLevel::Info, "offsets found");
            target.Reset();
            return offsets;
        }
    }

    Log(LogLevel::Error, "did not find all offsets");
    return std::nullopt;
}

f32 Sensitivity() { return process.Read<f32>(offsets.convar.sensitivity + 0x40); }

bool IsFfa() { return process.Read<u32>(offsets.convar.ffa + 0x40) == 1; }

bool EntityHasOwner(const u64 entity) {
    // h_pOwnerEntity is a handle, which is an int
    return process.Read<i32>(entity + offsets.controller.owner_entity) != -1;
}

std::optional<std::string> GetEntityType(const u64 entity) {
    const u64 name_pointer = process.Read<u64>(process.Read<u64>(entity + 0x10) + 0x20);
    if (name_pointer == 0) {
        return std::nullopt;
    }

    std::string name = process.ReadString(name_pointer);

    if (name.find("weapon_") != std::string::npos) {
        name = name.substr(7);
        return name;
    }

    return std::nullopt;
}

bool IsButtonPressed(KeyCode &button) {
    // what the actual fuck is happening here?
    const auto value = process.Read<u32>(
        offsets.interface.input + (((button >> 5) * 4) + offsets.direct.button_state));
    return ((value >> (button & 31)) & 1) != 0;
}

glm::vec2 TargetAngle(
    const glm::vec3 &eye_position, const glm::vec3 &position, const glm::vec2 &aim_punch) {
    const auto forward = glm::normalize(position - eye_position);
    auto angles = AnglesFromVector(forward) - aim_punch;
    Vec2Clamp(angles);
    return angles;
}

// 5.0 fov scale at 0 units, down to 1.0 at 500 units and above
f32 DistanceScale(f32 distance) {
    if (distance > 500.0f) {
        return 1.0f;
    } else {
        return 5.0f - (distance / 125.0f);
    }
}

std::string MapName() {
    const u64 map_name_ptr = process.Read<u64>(offsets.direct.game_types + 288);
    const std::string map_name = process.ReadString(map_name_ptr);
    return map_name.length() > 4 ? map_name.substr(4) : "";
}

bool FindTarget() {
    const auto local_player = Player::LocalPlayer();
    if (!local_player) {
        return false;
    }

    const u8 local_team = local_player->Team();
    if (local_team != TEAM_CT && local_team != TEAM_T) {
        return false;
    }

    const bool ffa = IsFfa();

    // note to self: forgetting to clear this caused such a retarded memory leak
    players.clear();
    for (u64 i = 1; i <= 64; i++) {
        const auto player = Player::Index(i);
        if (!player) {
            continue;
        }

        if (!player->IsValid()) {
            continue;
        }

        if (player->Equals(*local_player)) {
            target.local_pawn_index = i - 1;
        }

        players.push_back(*player);
    }

    if (players.size() == 0) {
        target.Reset();
        return false;
    }

    const WeaponClass weapon_class = local_player->GetWeaponClass();
    if (weapon_class == WeaponClass::Unknown || weapon_class == WeaponClass::Grenade ||
        weapon_class == WeaponClass::Grenade) {
        target.Reset();
        return true;
    }

    const auto view_angles = local_player->ViewAngles();
    const auto shots_fired = local_player->ShotsFired();
    glm::vec2 aim_punch = glm::vec2(0.0f);
    if (weapon_class != WeaponClass::Sniper) {
        const auto punch = local_player->AimPunch();
        if (glm::length(punch) < 0.001f && shots_fired > 0) {
            aim_punch = target.aim_punch;
        } else {
            aim_punch = punch;
        }
    }
    target.aim_punch = aim_punch;

    f32 smallest_fov = 360.0f;
    const auto eye_position = local_player->EyePosition();
    if (target.player) {
        if (!target.player->IsValid()) {
            target.Reset();
        }
    } else {
        target.Reset();
    }

    // update target player
    if (!IsButtonPressed(config.aimbot.hotkey) || !target.player) {
        for (auto player : players) {
            if (!ffa && local_team == player.Team()) {
                continue;
            }

            const auto head_position = player.BonePosition(Bones::BoneHead);
            const auto distance = glm::distance(eye_position, head_position);
            const auto angle = TargetAngle(eye_position, head_position, aim_punch);
            const f32 fov = AnglesToFov(view_angles, angle);

            if (fov < smallest_fov) {
                smallest_fov = fov;

                target.player = player;
                target.angle = angle;
                target.distance = distance;
                target.bone_index = Bones::BoneHead;
            }
        }
    }

    if (!target.player) {
        return true;
    }

    // update target angle
    smallest_fov = 360.0f;
    for (const auto bone : all_bones) {
        const auto bone_position = target.player->BonePosition(bone);
        const auto distance = glm::distance(eye_position, bone_position);
        const auto angle = TargetAngle(eye_position, bone_position, aim_punch);
        const auto fov = AnglesToFov(view_angles, angle);

        if (fov < smallest_fov) {
            smallest_fov = fov;

            target.angle = angle;
            target.distance = distance;
            target.bone_index = bone;
        }
    }

    return true;
}

void ClearVisualInfo() {
    vinfo_lock.lock();
    players.clear();
    entity_info.clear();
    vinfo_lock.unlock();
}

void VisualInfo() {
    vinfo_lock.lock();
    const auto local_player = Player::LocalPlayer();
    if (!local_player) {
        all_player_info.clear();
        enemy_info.clear();
        entity_info.clear();
        vinfo_lock.unlock();
        return;
    }
    const auto local_team = local_player->Team();
    const auto ffa = IsFfa();
    const auto spectated_player = local_player->SpectatorTarget();
    std::vector<PlayerInfo> player_info_all;
    std::vector<PlayerInfo> player_info_enemy;
    for (auto &player : players) {
        PlayerInfo info = {0};
        info.health = player.Health();
        info.armor = player.Armor();
        info.team = player.Team();
        info.position = player.Position();
        info.head = player.BonePosition(Bones::BoneHead);
        info.view_angles = player.ViewAngles();
        info.has_defuser = player.HasDefuser();
        info.has_helmet = player.HasHelmet();
        info.has_bomb = player.HasBomb();
        info.is_active = player.controller == local_player->controller ||
                         player.controller == spectated_player.value_or(0);
        info.name = player.Name();
        info.steam_id = player.SteamID();
        info.weapon = player.WeaponName();
        info.bones = player.AllBones();

        player_info_all.push_back(info);
        if (ffa || info.team != local_team) {
            player_info_enemy.push_back(info);
        }
    }

    // entities
    std::vector<EntityInfo> entity_info_new;
    for (u64 i = 64; i <= 1024; i++) {
        const auto entity = Player::ClientEntity(i);
        if (!entity) {
            continue;
        }

        // is a held weapon
        if (EntityHasOwner(*entity)) {
            continue;
        }

        const auto name = GetEntityType(*entity);
        if (!name) {
            continue;
        }

        const u64 gs_node = process.Read<u64>(*entity + offsets.pawn.game_scene_node);
        if (!gs_node) {
            continue;
        }
        const auto position = process.Read<glm::vec3>(gs_node + offsets.game_scene_node.origin);

        entity_info_new.push_back(EntityInfo{.name = *name, .position = position});
    }

    if (player_info_all.size() > 0) {
        all_player_info = player_info_all;
        enemy_info = player_info_enemy;
    } else {
        all_player_info.clear();
        enemy_info.clear();
    }

    if (entity_info_new.size() > 0) {
        entity_info = entity_info_new;
    } else {
        entity_info.clear();
    }

    view_matrix = process.Read<glm::mat4>(offsets.direct.view_matrix);
    const u64 sdl_window = process.Read<u64>(offsets.direct.sdl_window);
    if (sdl_window == 0) {
        window_size = glm::ivec4(0);
    } else {
        window_size = process.Read<glm::ivec4>(sdl_window + 0x18);
    }

    if (local_player) {
        misc_info.held_weapon = local_player->WeaponName();
    } else {
        misc_info.held_weapon = "?";
    }
    misc_info.is_ffa = IsFfa();
    misc_info.map_name = MapName();
    vinfo_lock.unlock();
}

void Run() {
    VisualInfo();
    if (!FindTarget()) {
        ClearVisualInfo();
        return;
    }

    FovChanger();
    NoFlash();
    Rcs();

    Aimbot();
    Triggerbot();
}
