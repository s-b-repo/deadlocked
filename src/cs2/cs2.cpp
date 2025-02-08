#include "cs2/cs2.hpp"

#include <string.h>

#include <iostream>

#include "config.hpp"
#include "cs2/constants.hpp"
#include "cs2/player.hpp"
#include "math.hpp"

bool is_valid = false;
Process process = {0};
Offsets offsets = {0};
Target target;
std::vector<Player> players;

extern Config config;
extern std::vector<PlayerInfo> player_info;
extern glm::mat4 view_matrix;
extern glm::ivec4 window_size;

void CS2() {
    if (IsValid()) {
        Run();
    } else {
        Setup();
    }
}

bool IsValid() {
    if (!process.pid) {
        return false;
    }
    return is_valid && ValidatePid(process.pid);
}

void Setup() {
    const std::optional<u64> pid_opt = GetPid(PROCESS_NAME);
    if (!pid_opt.has_value()) {
        is_valid = false;
        return;
    }
    const u64 pid = pid_opt.value();

    const std::optional<Process> process_opt = OpenProcess(pid);
    if (!process_opt.has_value()) {
        is_valid = false;
        return;
    }
    process = process_opt.value();

    const std::optional<Offsets> offsets_opt = FindOffsets();
    if (!offsets_opt.has_value()) {
        is_valid = false;
        return;
    }
    offsets = offsets_opt.value();

    is_valid = true;
}

std::optional<Offsets> FindOffsets() {
    Offsets offsets = {};

    // get library base addresses, will fail if game is not yet fully loaded
    const auto client_address = process.GetModuleBaseAddress(CLIENT_LIB);
    if (!client_address.has_value()) {
        return std::nullopt;
    }

    const auto engine_address = process.GetModuleBaseAddress(ENGINE_LIB);
    if (!engine_address.has_value()) {
        return std::nullopt;
    }

    const auto tier0_address = process.GetModuleBaseAddress(TIER0_LIB);
    if (!tier0_address.has_value()) {
        return std::nullopt;
    }

    const auto input_address = process.GetModuleBaseAddress(INPUT_LIB);
    if (!input_address.has_value()) {
        return std::nullopt;
    }

    const auto sdl_address = process.GetModuleBaseAddress(SDL_LIB);
    if (!sdl_address.has_value()) {
        return std::nullopt;
    }

    const auto matchmaking_address = process.GetModuleBaseAddress(MATCH_LIB);
    if (!matchmaking_address.has_value()) {
        return std::nullopt;
    }

    offsets.library.client = client_address.value();
    offsets.library.engine = engine_address.value();
    offsets.library.tier0 = tier0_address.value();
    offsets.library.input = input_address.value();
    offsets.library.sdl = sdl_address.value();
    offsets.library.matchmaking = matchmaking_address.value();

    // used for player interface offset
    const auto resource_offset = process.GetInterfaceOffset(offsets.library.engine, "GameResourceServiceClientV0");
    if (!resource_offset.has_value()) {
        std::cerr << "failed to get resource offset\n";
        return std::nullopt;
    }
    offsets.interface.resource = resource_offset.value();
    offsets.interface.entity = process.Read<u64>(offsets.interface.resource + 0x50);
    offsets.interface.player = offsets.interface.entity + 0x10;

    const auto local_player = process.ScanPattern(
        {0x48, 0x83, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0xC3},
        {true, true, true, false, false, false, false, true, true, true, true, true}, 12, offsets.library.client);
    if (!local_player.has_value()) {
        std::cerr << "failed to get local player offset\n";
        return std::nullopt;
    }
    offsets.direct.local_player = process.GetRelativeAddress(local_player.value(), 0x03, 0x08);

    const auto cvar_address = process.GetInterfaceOffset(offsets.library.tier0, "VEngineCvar0");
    if (!cvar_address.has_value()) {
        std::cerr << "failed to get cvar offset\n";
        return std::nullopt;
    }
    offsets.interface.cvar = cvar_address.value();

    const auto input_system_address = process.GetInterfaceOffset(offsets.library.input, "InputSystemVersion0");
    if (!input_system_address.has_value()) {
        std::cerr << "failed to get input offset\n";
        return std::nullopt;
    }
    offsets.interface.input = input_system_address.value();

    const auto view_matrix = process.ScanPattern(
        {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x0D},
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
    if (!view_matrix.has_value()) {
        std::cerr << "could not find view matrix offset\n";
        return std::nullopt;
    }
    offsets.direct.view_matrix = process.GetRelativeAddress(view_matrix.value() + 0x07, 0x03, 0x07);

    offsets.direct.button_state = process.Read<u32>(process.GetInterfaceFunction(offsets.interface.input, 19) + 0x14);

    const auto game_types = process.ScanPattern({0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x84, 0x00,
                                                 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x07},
                                                {
                                                    true, true,  true,  false, false, false, false, true, true, true,
                                                    true, false, false, false, false, false, true,  true, true,
                                                },
                                                19, offsets.library.matchmaking);

    const auto sdl_window_address = process.GetModuleExport(offsets.library.sdl, "SDL_GetKeyboardFocus");
    if (!sdl_window_address.has_value()) {
        std::cerr << "could not find sdl window offset\n";
    }
    const u64 sdl_window = process.GetRelativeAddress(sdl_window_address.value(), 0x02, 0x06);
    const u64 sdl_window2 = process.Read<u64>(sdl_window);
    offsets.direct.sdl_window = process.GetRelativeAddress(sdl_window2, 0x03, 0x07);

    const auto ffa_address = process.GetConvar(offsets.interface.cvar, "mp_teammates_are_enemies");
    if (!ffa_address.has_value()) {
        std::cerr << "could not get mp_tammates_are_enemies convar offset\n";
    }
    offsets.convar.ffa = ffa_address.value();
    const auto sensitivity_address = process.GetConvar(offsets.interface.cvar, "sensitivity");
    if (!sensitivity_address.has_value()) {
        std::cerr << "could not get sensitivity convar offset\n";
    }
    offsets.convar.sensitivity = sensitivity_address.value();

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
            network_enable_name_pointer = *(u64 *)(network_enable_name_pointer - base + client_dump);
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
            offsets.controller.name = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_hPawn")) {
            if (!network_enable || offsets.controller.pawn != 0) {
                continue;
            }
            offsets.controller.pawn = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_hOwnerEntity")) {
            if (!network_enable || offsets.controller.owner_entity != 0) {
                continue;
            }
            offsets.controller.owner_entity = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_iHealth")) {
            if (!network_enable || offsets.pawn.health != 0) {
                continue;
            }
            offsets.pawn.health = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_ArmorValue")) {
            if (!network_enable || offsets.pawn.armor != 0) {
                continue;
            }
            offsets.pawn.armor = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_iTeamNum")) {
            if (!network_enable || offsets.pawn.team != 0) {
                continue;
            }
            offsets.pawn.team = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_lifeState")) {
            if (!network_enable || offsets.pawn.life_state != 0) {
                continue;
            }
            offsets.pawn.life_state = *(i32 *)(entry + 0x08 + 0x10);
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
            offsets.pawn.eye_offset = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_aimPunchCache")) {
            if (!network_enable || offsets.pawn.aim_punch_cache != 0) {
                continue;
            }
            offsets.pawn.aim_punch_cache = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_iShotsFired")) {
            if (!network_enable || offsets.pawn.shots_fired != 0) {
                continue;
            }
            offsets.pawn.shots_fired = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("v_angle")) {
            if (offsets.pawn.view_angles != 0) {
                continue;
            }
            offsets.pawn.view_angles = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_entitySpottedState")) {
            if (!network_enable || offsets.pawn.spotted_state != 0) {
                continue;
            }
            const u64 offset = *(i32 *)(entry + 0x08 + 0x10);
            if (offset < 10000 || offset > 14000) {
                continue;
            }
            offsets.pawn.spotted_state = offset;
        } else if (name == std::string("m_pObserverServices")) {
            if (offsets.pawn.observer_services != 0) {
                continue;
            }
            offsets.pawn.observer_services = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_bDormant")) {
            if (offsets.game_scene_node.dormant != 0) {
                continue;
            }
            offsets.game_scene_node.dormant = *(i32 *)(entry + 0x08);
        } else if (name == std::string("m_vecAbsOrigin")) {
            if (!network_enable || offsets.game_scene_node.origin != 0) {
                continue;
            }
            offsets.game_scene_node.origin = *(i32 *)(entry + 0x08 + 0x10);
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
            offsets.spotted_state.mask = *(i32 *)(entry + 0x08 + 0x10);
        } else if (name == std::string("m_hObserverTarget")) {
            if (offsets.observer_service.target != 0) {
                continue;
            }
            offsets.observer_service.target = *(i32 *)(entry + 0x08);
        }

        if (offsets.AllFound()) {
            target.Reset();
            return offsets;
        }
    }

    std::cerr << "did not find all offsets\n";
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

    size_t position = std::string::npos;
    if (position = name.find("weapon_") != std::string::npos) {
        name = name.substr(7);
        return name;
    } else if (position = name.rfind("_projectile") != std::string::npos) {
        name = name.substr(0, name.length() - 11);
        if (name == std::string("incendiarygrenade")) {
            return std::string("incgrenade");
        }
        return name;
    }

    return std::nullopt;
}

glm::vec2 TargetAngle(Player &local_player, const glm::vec3 &eye_position, const glm::vec3 &position,
                      const glm::vec2 &aim_punch) {
    const auto forward = glm::normalize(position - eye_position);
    auto angles = AnglesFromVector(forward) - aim_punch;
    Vec2Clamp(angles);
    return angles;
}

void FindTarget() {
    const auto local_player_opt = Player::LocalPlayer();
    if (!local_player_opt.has_value()) {
        return;
    }
    Player local_player = local_player_opt.value();

    const u8 local_team = local_player.Team();
    if (local_team != TEAM_CT && local_team != TEAM_T) {
        return;
    }

    // note to self: forgetting to clear this caused such a retarded memory leak
    players.clear();
    for (u64 i = 1; i <= 64; i++) {
        const auto player_opt = Player::Index(i);
        if (!player_opt.has_value()) {
            continue;
        }
        Player player = player_opt.value();

        if (!player.IsValid()) {
            continue;
        }

        if (player.Equals(local_player)) {
            target.local_pawn_index = i - 1;
        }

        const u8 team = player.Team();
        if (team == local_team) {
            continue;
        }

        players.push_back(player);
    }

    const WeaponClass weapon_class = local_player.GetWeaponClass();
    if (weapon_class == WeaponClass::Unknown || weapon_class == WeaponClass::Grenade ||
        weapon_class == WeaponClass::Grenade) {
        target.Reset();
        return;
    }

    const auto view_angles = local_player.ViewAngles();
    const auto ffa = IsFfa();
    const auto shots_fired = local_player.ShotsFired();
    glm::vec2 aim_punch = glm::vec2(0.0);
    if (weapon_class != WeaponClass::Sniper) {
        const auto punch = local_player.AimPunch();
        if (glm::length(punch) < 0.001 && shots_fired > 0) {
            aim_punch = target.previous_aim_punch;
        } else {
            aim_punch = punch;
        }
    }
    target.previous_aim_punch = aim_punch;

    if (players.size() == 0) {
        target.Reset();
        return;
    }

    f32 smallest_fov = 360.0;
    const auto eye_position = local_player.EyePosition();
    if (target.player.has_value()) {
        if (!target.player.value().IsValid()) {
            target.Reset();
        }
    } else {
        target.Reset();
    }
    for (auto player : players) {
        if (!ffa && local_team == player.Team()) {
            continue;
        }

        const auto head_position = player.BonePosition(Bones::BoneHead);
        const auto distance = glm::distance(eye_position, head_position);
        const auto angle = TargetAngle(local_player, eye_position, head_position, aim_punch);
        const f32 fov = AnglesToFov(view_angles, angle);

        if (fov < smallest_fov) {
            smallest_fov = fov;

            target.player = player;
            target.angle = angle;
            target.distance = distance;
            target.bone_index = Bones::BoneHead;
        }
    }

    if (!target.player.has_value()) {
        return;
    }

    // update target angle
    smallest_fov = 360.0;
    auto target_player = target.player.value();
    for (const auto bone : all_bones) {
        const auto bone_position = target_player.BonePosition(bone);
        const auto distance = glm::distance(eye_position, bone_position);
        const auto angle = TargetAngle(local_player, eye_position, bone_position, aim_punch);
        const auto fov = AnglesToFov(view_angles, angle);

        if (fov < smallest_fov) {
            smallest_fov = fov;

            target.angle = angle;
            target.distance = distance;
            target.bone_index = bone;
        }
    }
}

void VisualInfo() {
    std::vector<PlayerInfo> player_info_new;
    for (auto &player : players) {
        PlayerInfo info = {0};
        info.health = player.Health();
        info.armor = player.Armor();
        info.team = player.Team();
        info.position = player.Position();
        info.head = player.BonePosition(Bones::BoneHead);
        info.weapon = player.WeaponName();
        info.bones = player.AllBones();

        const i32 spotted_mask = player.SpottedMask();
        info.visible = spotted_mask & (1 << target.local_pawn_index);

        player_info_new.push_back(info);
    }

    if (player_info_new.size() > 0) {
        player_info = player_info_new;
    } else {
        player_info.clear();
    }
    view_matrix = process.Read<glm::mat4>(offsets.direct.view_matrix);
    const u64 sdl_window = process.Read<u64>(offsets.direct.sdl_window);
    if (sdl_window == 0) {
        window_size = glm::ivec4(0);
    } else {
        window_size = process.Read<glm::ivec4>(sdl_window + 0x18);
    }
}

void Run() {
    FindTarget();
    VisualInfo();
}
