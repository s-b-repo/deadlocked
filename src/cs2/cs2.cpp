#include "cs2/cs2.hpp"

#include <string.h>

#include <iostream>

#include "config.hpp"
#include "cs2/constants.hpp"
#include "math.hpp"

std::optional<Offsets> FindOffsets();
u64 GetLocalController();
std::optional<u64> GetClientEntity(const u64 index);
std::optional<u64> GetPawn(const u64 controller);
std::string GetPlayerName(const u64 controller);
i32 GetHealth(const u64 pawn);
i32 GetArmor(const u64 pawn);
u8 GetTeam(const u64 pawn);
u8 GetLifeState(const u64 pawn);
std::string GetWeaponName(const u64 pawn);
u64 GameSceneNode(const u64 pawn);
bool IsDormant(const u64 pawn);
glm::vec3 GetPosition(const u64 pawn);
glm::vec3 GetEyePosition(const u64 pawn);
glm::vec3 GetBonePosition(const u64 pawn, const u64 bone_index);
i32 GetShotsFired(const u64 pawn);
f32 GetFovMultiplier(const u64 pawn);
i32 GetSpottedMask(const u64 pawn);
std::vector<std::pair<glm::vec3, glm::vec3>> GetBones(const u64 pawn);
bool IsPawnValid(const u64 pawn);
glm::vec2 GetViewAngles(const u64 pawn);
glm::vec2 GetAimPunch(const u64 pawn);
f32 GetSensitivity();
bool IsFfa();
bool EntityHasOwner(const u64 entity);
std::optional<std::string> GetEntityType(const u64 entity);

bool is_valid = false;
Process process = {0};
Offsets offsets = {0};

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

void Run() {
    const u64 local_controller = GetLocalController();
    const std::optional<u64> local_pawn_opt = GetPawn(local_controller);
    if (!local_pawn_opt.has_value()) {
        return;
    }
    const u64 local_pawn = local_pawn_opt.value();

    const u8 local_team = GetTeam(local_pawn);
    if (local_team != TEAM_CT && local_team != TEAM_T) {
        return;
    }

    std::vector<u64> pawns;
    std::vector<PlayerInfo> players;
    u64 local_pawn_index = 0;
    for (u64 i = 1; i <= 64; i++) {
        const std::optional<u64> controller_opt = GetClientEntity(i);
        if (!controller_opt.has_value()) {
            continue;
        }
        const u64 controller = controller_opt.value();

        const std::optional<u64> pawn_opt = GetPawn(controller);
        if (!pawn_opt.has_value()) {
            continue;
        }
        const u64 pawn = pawn_opt.value();

        if (!IsPawnValid(pawn)) {
            continue;
        }

        if (pawn == local_pawn) {
            local_pawn_index = i - 1;
        }

        const u8 team = GetTeam(pawn);
        if (team == local_team) {
            continue;
        }

        PlayerInfo player = {0};
        player.pawn = pawn;
        player.health = GetHealth(pawn);
        player.armor = GetArmor(pawn);
        player.team = GetTeam(pawn);
        player.position = GetPosition(pawn);
        player.head = GetBonePosition(pawn, Bones::BoneHead);
        player.visible = false;
        player.weapon = GetWeaponName(pawn);
        player.bones = GetBones(pawn);

        players.push_back(player);
    }

    for (auto &player : players) {
        const i32 spotted_mask = GetSpottedMask(player.pawn);
        player.visible = spotted_mask & (1 << local_pawn_index);
    }

    if (players.size() > 0) {
        player_info = players;
    } else {
        player_info.clear();
    }
    view_matrix = ReadMat4(&process, offsets.direct.view_matrix);
    const u64 sdl_window = ReadU64(&process, offsets.direct.sdl_window);
    if (sdl_window == 0) {
        window_size = glm::ivec4(0);
    } else {
        window_size = ReadIVec4(&process, sdl_window + 0x18);
    }
}

std::optional<Offsets> FindOffsets() {
    Offsets offsets = {};

    // get library base addresses, will fail if game is not yet fully loaded
    const auto client_address = GetModuleBaseAddress(&process, CLIENT_LIB);
    if (!client_address.has_value()) {
        return std::nullopt;
    }

    const auto engine_address = GetModuleBaseAddress(&process, ENGINE_LIB);
    if (!engine_address.has_value()) {
        return std::nullopt;
    }

    const auto tier0_address = GetModuleBaseAddress(&process, TIER0_LIB);
    if (!tier0_address.has_value()) {
        return std::nullopt;
    }

    const auto input_address = GetModuleBaseAddress(&process, INPUT_LIB);
    if (!input_address.has_value()) {
        return std::nullopt;
    }

    const auto sdl_address = GetModuleBaseAddress(&process, SDL_LIB);
    if (!sdl_address.has_value()) {
        return std::nullopt;
    }

    const auto matchmaking_address = GetModuleBaseAddress(&process, MATCH_LIB);
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
    const auto resource_offset =
        GetInterfaceOffset(&process, offsets.library.engine, "GameResourceServiceClientV0");
    if (!resource_offset.has_value()) {
        std::cerr << "failed to get resource offset\n";
        return std::nullopt;
    }
    offsets.interface.resource = resource_offset.value();
    offsets.interface.entity = ReadU64(&process, offsets.interface.resource + 0x50);
    offsets.interface.player = offsets.interface.entity + 0x10;

    const auto local_player = ScanPattern(
        &process, {0x48, 0x83, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0xC3},
        {true, true, true, false, false, false, false, true, true, true, true, true}, 12,
        offsets.library.client);
    if (!local_player.has_value()) {
        std::cerr << "failed to get local player offset\n";
        return std::nullopt;
    }
    offsets.direct.local_player = GetRelativeAddress(&process, local_player.value(), 0x03, 0x08);

    const auto cvar_address = GetInterfaceOffset(&process, offsets.library.tier0, "VEngineCvar0");
    if (!cvar_address.has_value()) {
        std::cerr << "failed to get cvar offset\n";
        return std::nullopt;
    }
    offsets.interface.cvar = cvar_address.value();

    const auto input_system_address =
        GetInterfaceOffset(&process, offsets.library.input, "InputSystemVersion0");
    if (!input_system_address.has_value()) {
        std::cerr << "failed to get input offset\n";
        return std::nullopt;
    }
    offsets.interface.input = input_system_address.value();

    const auto view_matrix = ScanPattern(&process,
                                         {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x8D,
                                          0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x0D},
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
    offsets.direct.view_matrix =
        GetRelativeAddress(&process, view_matrix.value() + 0x07, 0x03, 0x07);

    offsets.direct.button_state =
        ReadU32(&process, GetInterfaceFunction(&process, offsets.interface.input, 19) + 0x14);

    const auto game_types =
        ScanPattern(&process,
                    {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x84, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x48, 0x8B, 0x07},
                    {
                        true, true,  true,  false, false, false, false, true, true, true,
                        true, false, false, false, false, false, true,  true, true,
                    },
                    19, offsets.library.matchmaking);

    const auto sdl_window_address =
        GetModuleExport(&process, offsets.library.sdl, "SDL_GetKeyboardFocus");
    if (!sdl_window_address.has_value()) {
        std::cerr << "could not find sdl window offset\n";
    }
    const u64 sdl_window = GetRelativeAddress(&process, sdl_window_address.value(), 0x02, 0x06);
    const u64 sdl_window2 = ReadU64(&process, sdl_window);
    offsets.direct.sdl_window = GetRelativeAddress(&process, sdl_window2, 0x03, 0x07);

    // crosshair alpha technically not needed, keep around for future
    const auto ffa_address =
        GetConvar(&process, offsets.interface.cvar, "mp_teammates_are_enemies");
    if (!ffa_address.has_value()) {
        std::cerr << "could not get mp_tammates_are_enemies convar offset\n";
    }
    offsets.convar.ffa = ffa_address.value();
    const auto sensitivity_address = GetConvar(&process, offsets.interface.cvar, "sensitivity");
    if (!sensitivity_address.has_value()) {
        std::cerr << "could not get sensitivity convar offset\n";
    }
    offsets.convar.sensitivity = sensitivity_address.value();

    // dump all netvars from client lib
    const u64 base = offsets.library.client;
    const u64 size = ModuleSize(&process, base);

    const std::vector<u8> client_dump_vec = DumpModule(&process, base);
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
        } else if (name == std::string("m_vecVelocity")) {
            if (offsets.pawn.velocity != 0) {
                continue;
            }
            const u64 offset = *(i32 *)(entry + 0x08);
            if (offset < 800 || offset > 1600) {
                continue;
            }
            offsets.pawn.velocity = offset;
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
            return offsets;
        }
    }

    std::cerr << "did not find all offsets\n";
    return std::nullopt;
}

u64 GetLocalController() { return ReadU64(&process, offsets.direct.local_player); }

std::optional<u64> GetClientEntity(const u64 index) {
    // wtf is this doing, and how?
    const u64 v1 = ReadU64(&process, offsets.interface.entity + 0x08 * (index >> 9) + 0x10);
    if (v1 == 0) {
        return std::nullopt;
    }
    // what?
    const u64 entity = ReadU64(&process, v1 + 120 * (index & 0x1ff));
    if (entity == 0) {
        return std::nullopt;
    }

    return entity;
}

std::optional<u64> GetPawn(const u64 controller) {
    const u64 v1 = ReadI32(&process, controller + offsets.controller.pawn);
    if (v1 == -1) {
        return std::nullopt;
    }

    // what the fuck is this doing?
    const u64 v2 = ReadU64(&process, offsets.interface.player + 8 * ((v1 & 0x7fff) >> 9));
    if (v2 == 0) {
        return std::nullopt;
    }

    // bit-fuckery, why is this needed exactly?
    const u64 entity = ReadU64(&process, v2 + 120 * (v1 & 0x1ff));
    if (entity == 0) {
        return std::nullopt;
    }

    return entity;
}

std::string GetPlayerName(const u64 controller) {
    const u64 name_address = ReadU64(&process, controller + offsets.controller.name);
    if (name_address == 0) {
        return std::string("?");
    }

    return ReadString(&process, name_address);
}

i32 GetHealth(const u64 pawn) {
    const i32 health = ReadI32(&process, pawn + offsets.pawn.health);
    if (health <= 0 || health > 100) {
        return 0;
    }

    return health;
}

i32 GetArmor(const u64 pawn) {
    const i32 armor = ReadI32(&process, pawn + offsets.pawn.armor);
    if (armor < 0 || armor > 100) {
        return 0;
    }

    return armor;
}

u8 GetTeam(const u64 pawn) { return ReadU8(&process, pawn + offsets.pawn.team); }

u8 GetLifeState(const u64 pawn) { return ReadU8(&process, pawn + offsets.pawn.life_state); }

std::string GetWeaponName(const u64 pawn) {
    // CEntityInstance
    const u64 weapon_entity_instance = ReadU64(&process, pawn + offsets.pawn.weapon);
    if (weapon_entity_instance == 0) {
        return std::string("?");
    }
    // CEntityIdentity, 0x10 = m_pEntity
    const u64 weapon_entity_identity = ReadU64(&process, weapon_entity_instance + 0x10);
    if (weapon_entity_identity == 0) {
        return std::string("?");
    }
    // 0x20 = m_designerName (pointer -> string)
    const u64 weapon_name_pointer = ReadU64(&process, weapon_entity_identity + 0x20);
    if (weapon_name_pointer == 0) {
        return std::string("?");
    }

    return ReadString(&process, weapon_name_pointer);
}

u64 GameSceneNode(const u64 pawn) { return ReadU64(&process, pawn + offsets.pawn.game_scene_node); }

bool IsDormant(const u64 pawn) {
    const u64 gs_node = GameSceneNode(pawn);
    return ReadU8(&process, gs_node + offsets.game_scene_node.dormant) != 0;
}

glm::vec3 GetPosition(const u64 pawn) {
    const u64 gs_node = GameSceneNode(pawn);
    return ReadVec3(&process, gs_node + offsets.game_scene_node.origin);
}

glm::vec3 GetEyePosition(const u64 pawn) {
    const glm::vec3 position = GetPosition(pawn);
    const glm::vec3 eye_offset = ReadVec3(&process, pawn + offsets.pawn.eye_offset);

    return position + eye_offset;
}

glm::vec3 GetBonePosition(const u64 pawn, const u64 bone_index) {
    const u64 gs_node = GameSceneNode(pawn);
    const u64 bone_data = ReadU64(&process, gs_node + offsets.game_scene_node.model_state + 0x80);

    if (bone_data == 0) {
        return glm::vec3(0.0);
    }

    return ReadVec3(&process, bone_data + (bone_index * 32));
}

i32 GetShotsFired(const u64 pawn) { return ReadI32(&process, pawn + offsets.pawn.shots_fired); }

f32 GetFovMultiplier(const u64 pawn) {
    return ReadF32(&process, pawn + offsets.pawn.fov_multiplier);
}

i32 GetSpottedMask(const u64 pawn) {
    return ReadI32(&process, pawn + offsets.pawn.spotted_state + offsets.spotted_state.mask);
}

std::vector<std::pair<glm::vec3, glm::vec3>> GetBones(const u64 pawn) {
    std::unordered_map<Bones, glm::vec3> bones;

    for (const Bones bone : all_bones) {
        const glm::vec3 position = GetBonePosition(pawn, bone);
        bones.insert({bone, position});
    }

    std::vector<std::pair<glm::vec3, glm::vec3>> connections(bone_connections.size());

    i32 i = 0;
    for (const auto connection : bone_connections) {
        connections[i] = {bones.at(connection.first), bones.at(connection.second)};
        i++;
    }

    return connections;
}

bool IsPawnValid(const u64 pawn) {
    if (IsDormant(pawn)) {
        return false;
    }

    if (GetHealth(pawn) <= 0) {
        return false;
    }

    if (GetLifeState(pawn) != 0) {
        return false;
    }

    return true;
}

glm::vec2 GetViewAngles(const u64 pawn) {
    return ReadVec2(&process, pawn + offsets.pawn.view_angles);
}

glm::vec2 GetAimPunch(const u64 pawn) {
    const u64 length = ReadU64(&process, pawn + offsets.pawn.aim_punch_cache);
    if (length < 1) {
        return glm::vec2(0.0);
    }

    const u64 data_address = ReadU64(&process, pawn + offsets.pawn.aim_punch_cache + 0x08);

    return ReadVec2(&process, data_address + (length - 1) * 12);
}

f32 GetSensitivity() { return ReadF32(&process, offsets.convar.sensitivity + 0x40); }

bool IsFfa() { return ReadU32(&process, offsets.convar.ffa + 0x40) == 1; }

bool EntityHasOwner(const u64 entity) {
    // h_pOwnerEntity is a handle, which is an int
    return ReadI32(&process, entity + offsets.controller.owner_entity) != -1;
}

std::optional<std::string> GetEntityType(const u64 entity) {
    const u64 name_pointer = ReadU64(&process, ReadU64(&process, entity + 0x10) + 0x20);
    if (name_pointer == 0) {
        return std::nullopt;
    }

    std::string name = ReadString(&process, name_pointer);

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
