#include "game.h"

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "weapons.h"

bool find_offsets(const ProcessHandle *process, Offsets *offsets) {
    const u64 client_library = get_library_base_offset(process, CLIENT_LIB);
    const u64 engine_library = get_library_base_offset(process, ENGINE_LIB);
    const u64 tier0_library = get_library_base_offset(process, TIER0_LIB);
    const u64 input_library = get_library_base_offset(process, INPUT_LIB);
    const u64 sdl_library = get_library_base_offset(process, SDL_LIB);
    if (!client_library || !engine_library || !tier0_library || !input_library || !sdl_library) {
        printf("could not locate library offsets\n");
        return false;
    }
    offsets->library.client = client_library;
    offsets->library.engine = engine_library;
    offsets->library.tier0 = tier0_library;
    offsets->library.input = input_library;
    offsets->library.sdl = sdl_library;

    const u64 resource_interface = get_interface(process, offsets->library.engine, "GameResourceServiceClientV0");
    if (!resource_interface) {
        return false;
    }
    offsets->interface.resource = resource_interface;

    offsets->interface.entity = read_u64(process, offsets->interface.resource + 0x50);
    offsets->interface.player = offsets->interface.entity + 0x10;

    offsets->interface.convar = get_interface(process, offsets->library.tier0, "VEngineCvar0");
    offsets->interface.input = get_interface(process, offsets->library.input, "InputSystemVersion0");

    // some inexplicable black magic
    offsets->direct.button_state =
        read_u32(process, get_interface_function(process, offsets->interface.input, 19) + 0x14);

    const u64 local_controller =
        scan_pattern(process, offsets->library.client, (u8 *)"\x48\x83\x3D\x00\x00\x00\x00\x00\x0F\x95\xC0\xC3",
                     (u8 *)"xxx????xxxxx", 12);
    if (!local_controller) {
        printf("could not find local player controller\n");
        return false;
    }
    // there was 0x07 instead of 0x08, FUUUCK
    offsets->direct.local_controller = get_relative_address(process, local_controller, 0x03, 0x08);

    offsets->convars.sensitivity = get_convar(process, offsets->interface.convar, "sensitivity");
    offsets->convars.ffa = get_convar(process, offsets->interface.convar, "mp_teammates_are_enemies");

    // dump netvars
    const u8 *client_dump = dump_library(process, offsets->library.client);
    if (!client_dump) {
        printf("could not dump client library\n");
        return false;
    }

    const u64 base = offsets->library.client;
    const u64 size = *((u64 *)client_dump - 1);

    for (u64 i = size - sizeof(u64); i > 0; i -= 8) {
        const u64 entry = ((u64)client_dump + i);
        bool network_enable = false;
        u64 ne_name_pointer = *(u64 *)entry;

        if (ne_name_pointer == 0) {
            continue;
        }

        // can this be rewritten to be more concise?
        if (ne_name_pointer >= base && ne_name_pointer <= base + size) {
            ne_name_pointer = *(u64 *)(ne_name_pointer - base + client_dump);
            if (ne_name_pointer >= base && ne_name_pointer <= base + size) {
                const char *name = (char *)(ne_name_pointer - base + client_dump);
                if (strncmp(name, "MNetworkEnable", 14) == 0) {
                    network_enable = true;
                }
            }
        }

        u64 name_pointer = 0;
        if (network_enable) {
            name_pointer = *(u64 *)(entry + 0x08);
        } else {
            name_pointer = *(u64 *)(entry);
        }
        if (name_pointer < base || name_pointer > base + size) {
            continue;
        }

        const char *name = (char *)(name_pointer - base + client_dump);

        // switch
        if (!strcmp(name, "m_hPawn")) {
            if (!network_enable || offsets->controller.pawn) {
                continue;
            }
            offsets->controller.pawn = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_iHealth")) {
            if (!network_enable || offsets->pawn.health) {
                continue;
            }
            offsets->pawn.health = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_iTeamNum")) {
            if (!network_enable || offsets->pawn.team) {
                continue;
            }
            offsets->pawn.team = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_lifeState")) {
            if (!network_enable || offsets->pawn.life_state) {
                continue;
            }
            offsets->pawn.life_state = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_pClippingWeapon")) {
            if (offsets->pawn.weapon) {
                continue;
            }
            offsets->pawn.weapon = *(u32 *)(entry + 0x10);
        } else if (!strcmp(name, "m_flFOVSensitivityAdjust")) {
            if (offsets->pawn.fov_multiplier) {
                continue;
            }
            offsets->pawn.fov_multiplier = *(u32 *)(entry + 0x08);
        } else if (!strcmp(name, "m_pGameSceneNode")) {
            if (offsets->pawn.game_scene_node) {
                continue;
            }
            offsets->pawn.game_scene_node = *(u32 *)(entry + 0x10);
        } else if (!strcmp(name, "m_vecViewOffset")) {
            if (!network_enable || offsets->pawn.eye_offset) {
                continue;
            }
            offsets->pawn.eye_offset = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_aimPunchCache")) {
            if (!network_enable || offsets->pawn.aim_punch_cache) {
                continue;
            }
            offsets->pawn.aim_punch_cache = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_iShotsFired")) {
            if (!network_enable || offsets->pawn.shots_fired) {
                continue;
            }
            offsets->pawn.shots_fired = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "v_angle")) {
            if (offsets->pawn.view_angles) {
                continue;
            }
            offsets->pawn.view_angles = *(u32 *)(entry + 0x08);
        } else if (!strcmp(name, "m_bDormant")) {
            if (offsets->game_scene_node.dormant) {
                continue;
            }
            offsets->game_scene_node.dormant = *(u32 *)(entry + 0x08);
        } else if (!strcmp(name, "m_vecAbsOrigin")) {
            if (!network_enable || offsets->game_scene_node.origin) {
                continue;
            }
            offsets->game_scene_node.origin = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strcmp(name, "m_modelState")) {
            if (offsets->game_scene_node.model_state) {
                continue;
            }
            offsets->game_scene_node.model_state = *(u32 *)(entry + 0x08);
        }

        if (all_offsets_found(offsets)) {
            return true;
        }
    }
    if (!all_offsets_found(offsets)) {
        printf("not all offsets found\n");
        return false;
    }

    return true;
}

Vec2 get_view_angles(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    const Vec2 angles = read_vec2(process, pawn + offsets->pawn.view_angles);
    return angles;
}

u64 get_local_controller(const ProcessHandle *process, const Offsets *offsets) {
    return read_u64(process, offsets->direct.local_controller);
}

u64 get_client_entity(const ProcessHandle *process, const Offsets *offsets, const u64 index) {
    // what the fuck?
    const u64 v2 = read_u64(process, offsets->interface.entity + 8 * (index >> 9) + 16);
    if (v2 == 0) return 0;

    return read_u64(process, (u64)(120 * (index & 0x1FF) + v2));
}

u64 get_pawn(const ProcessHandle *process, const Offsets *offsets, const u64 controller) {
    // this HAS to be i32, WHYY
    const u64 v1 = read_i32(process, controller + offsets->controller.pawn);
    if (v1 == -1) {
        return 0;
    }

    // wtf is this?
    const u64 v2 = read_u64(process, offsets->interface.player + 8 * ((v1 & 0x7FFF) >> 9));
    if (v2 == 0) {
        return 0;
    }
    return read_u64(process, v2 + 120 * (v1 & 0x1FF));
}

f32 get_sensitivity(const ProcessHandle *process, const Offsets *offsets) {
    return read_f32(process, offsets->convars.sensitivity + 0x40);
}

bool is_ffa(const ProcessHandle *process, const Offsets *offsets) {
    return read_u32(process, offsets->convars.ffa + 0x40) == 1;
}

bool is_button_down(const ProcessHandle *process, const Offsets *offsets, const u64 button) {
    // what the actual fuck is happening here?
    const u64 value =
        read_u32(process, (offsets->interface.input + (((button >> 5) * 4) + offsets->direct.button_state)));
    return (value >> (button & 31)) & 1;
}

i32 get_health(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    const i32 health = read_i32(process, pawn + offsets->pawn.health);
    if (health < 0 || health > 100) {
        return 0;
    }
    return health;
}

u8 get_team(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    return read_u8(process, pawn + offsets->pawn.team);
}

u8 get_life_state(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    return read_u8(process, pawn + offsets->pawn.life_state);
}

u64 get_gs_node(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    return read_u64(process, pawn + offsets->pawn.game_scene_node);
}

bool is_dormant(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    const u64 gs_node = get_gs_node(process, offsets, pawn);
    return read_u8(process, gs_node + offsets->game_scene_node.dormant);
}

Vec3 get_position(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    const u64 game_scene_node = get_gs_node(process, offsets, pawn);
    return read_vec3(process, game_scene_node + offsets->game_scene_node.origin);
}

Vec3 get_eye_position(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    Vec3 position = get_position(process, offsets, pawn);
    const Vec3 eye_offset = read_vec3(process, pawn + offsets->pawn.eye_offset);

    position.x += eye_offset.x;
    position.y += eye_offset.y;
    position.z += eye_offset.z;

    return position;
}

Vec3 get_bone_position(const ProcessHandle *process, const Offsets *offsets, const u64 pawn, const u64 bone_index) {
    const u64 gs_node = get_gs_node(process, offsets, pawn);
    const u64 bone_data = read_u64(process, gs_node + offsets->game_scene_node.model_state + 0x80);

    if (bone_data == 0) {
        return (Vec3){.x = 0.0, .y = 0.0, .z = 0.0};
    }

    const Vec3 position = read_vec3(process, bone_data + (bone_index * 32));
    return position;
}

i32 get_shots_fired(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    return read_i32(process, pawn + offsets->pawn.shots_fired);
}

f32 get_fov_multiplier(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    return read_f32(process, pawn + offsets->pawn.fov_multiplier);
}

Vec2 get_aim_punch(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    const u64 length = read_u64(process, pawn + offsets->pawn.aim_punch_cache);
    if (length < 1) {
        return (Vec2){.x = 0.0, .y = 0.0};
    }

    const u64 data_address = read_u64(process, pawn + offsets->pawn.aim_punch_cache + sizeof(u64));

    const Vec2 angle = read_vec2(process, data_address + (length - 1) * 12);

    return angle;
}

// has to be freed!
char *get_weapon(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    const u64 weapon_entity_instance = read_u64(process, pawn + offsets->pawn.weapon);
    if (weapon_entity_instance == 0) {
        return "unknown";
    }

    const u64 weapon_entity_identity = read_u64(process, weapon_entity_instance + 0x10);
    if (weapon_entity_identity == 0) {
        return "unknown";
    }

    const u64 weapon_name_pointer = read_u64(process, weapon_entity_identity + 0x20);
    if (weapon_name_pointer == 0) {
        return "unknown";
    }

    return read_string(process, weapon_name_pointer);
}

enum WeaponClass get_weapon_class(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    char *weapon = get_weapon(process, offsets, pawn);
    // Knives
    if (!strcmp(weapon, "weapon_bayonet") || !strcmp(weapon, "weapon_knife") || !strcmp(weapon, "weapon_knife_bowie") ||
        !strcmp(weapon, "weapon_knife_butterfly") || !strcmp(weapon, "weapon_knife_canis") ||
        !strcmp(weapon, "weapon_knife_cord") || !strcmp(weapon, "weapon_knife_css") ||
        !strcmp(weapon, "weapon_knife_falchion") || !strcmp(weapon, "weapon_knife_flip") ||
        !strcmp(weapon, "weapon_knife_gut") || !strcmp(weapon, "weapon_knife_gypsy_jackknife") ||
        !strcmp(weapon, "weapon_knife_karambit") || !strcmp(weapon, "weapon_knife_kukri") ||
        !strcmp(weapon, "weapon_knife_m9_bayonet") || !strcmp(weapon, "weapon_knife_outdoor") ||
        !strcmp(weapon, "weapon_knife_push") || !strcmp(weapon, "weapon_knife_skeleton") ||
        !strcmp(weapon, "weapon_knife_stiletto") || !strcmp(weapon, "weapon_knife_survival_bowie") ||
        !strcmp(weapon, "weapon_knife_t") || !strcmp(weapon, "weapon_knife_tactical") ||
        !strcmp(weapon, "weapon_knife_twinblade") || !strcmp(weapon, "weapon_knife_ursus") ||
        !strcmp(weapon, "weapon_knife_widowmaker")) {
        return WEAPON_CLASS_KNIFE;
    }

    // Pistols
    if (!strcmp(weapon, "weapon_cz75a") || !strcmp(weapon, "weapon_deagle") || !strcmp(weapon, "weapon_elite") ||
        !strcmp(weapon, "weapon_fiveseven") || !strcmp(weapon, "weapon_glock") || !strcmp(weapon, "weapon_hkp2000") ||
        !strcmp(weapon, "weapon_p2000") || !strcmp(weapon, "weapon_p250") || !strcmp(weapon, "weapon_revolver") ||
        !strcmp(weapon, "weapon_tec9") || !strcmp(weapon, "weapon_usp_silencer") ||
        !strcmp(weapon, "weapon_usp_silencer_off")) {
        return WEAPON_CLASS_PISTOL;
    }

    // SMGs
    if (!strcmp(weapon, "weapon_bizon") || !strcmp(weapon, "weapon_mac10") || !strcmp(weapon, "weapon_mp5sd") ||
        !strcmp(weapon, "weapon_mp7") || !strcmp(weapon, "weapon_mp9") || !strcmp(weapon, "weapon_p90") ||
        !strcmp(weapon, "weapon_ump45")) {
        return WEAPON_CLASS_SMG;
    }

    // Heavy weapons (Shotguns & LMGs)
    if (!strcmp(weapon, "weapon_m249") || !strcmp(weapon, "weapon_negev") || !strcmp(weapon, "weapon_mag7") ||
        !strcmp(weapon, "weapon_nova") || !strcmp(weapon, "weapon_sawedoff") || !strcmp(weapon, "weapon_xm1014")) {
        return WEAPON_CLASS_HEAVY;
    }

    // Rifles
    if (!strcmp(weapon, "weapon_ak47") || !strcmp(weapon, "weapon_aug") || !strcmp(weapon, "weapon_famas") ||
        !strcmp(weapon, "weapon_galilar") || !strcmp(weapon, "weapon_m4a1_silencer") ||
        !strcmp(weapon, "weapon_m4a1_silencer_off") || !strcmp(weapon, "weapon_m4a1") ||
        !strcmp(weapon, "weapon_sg556")) {
        return WEAPON_CLASS_RIFLE;
    }

    // Snipers
    if (!strcmp(weapon, "weapon_awp") || !strcmp(weapon, "weapon_g3sg1") || !strcmp(weapon, "weapon_scar20") ||
        !strcmp(weapon, "weapon_ssg08")) {
        return WEAPON_CLASS_SNIPER;
    }

    // Grenades
    if (!strcmp(weapon, "weapon_decoy") || !strcmp(weapon, "weapon_firebomb") || !strcmp(weapon, "weapon_flashbang") ||
        !strcmp(weapon, "weapon_frag_grenade") || !strcmp(weapon, "weapon_hegrenade") ||
        !strcmp(weapon, "weapon_incgrenade") || !strcmp(weapon, "weapon_molotov") ||
        !strcmp(weapon, "weapon_smokegrenade")) {
        return WEAPON_CLASS_GRENADE;
    }

    // Utility
    if (!strcmp(weapon, "weapon_taser")) {
        return WEAPON_CLASS_UTILITY;
    }

    // Default case: unknown weapon
    printf("unknown weapon: %s", weapon);
    return WEAPON_CLASS_UNKNOWN;
}

bool is_pawn_valid(const ProcessHandle *process, const Offsets *offsets, const u64 pawn) {
    if (is_dormant(process, offsets, pawn)) {
        return false;
    }

    if (get_health(process, offsets, pawn) <= 0) {
        return false;
    }

    if (get_life_state(process, offsets, pawn)) {
        return false;
    }

    return true;
}
