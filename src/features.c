#include "features.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "constants.h"

bool find_offsets(ProcessHandle *process, Offsets *offsets) {
    const u64 client_library = get_library_base_offset(process, CLIENT_LIB);
    const u64 engine_library = get_library_base_offset(process, ENGINE_LIB);
    const u64 tier0_library = get_library_base_offset(process, TIER0_LIB);
    const u64 input_library = get_library_base_offset(process, INPUT_LIB);
    const u64 sdl_library = get_library_base_offset(process, SDL_LIB);
    if (!client_library || !engine_library || !tier0_library ||
        !input_library || !sdl_library) {
        return false;
    }
    offsets->library.client = client_library;
    offsets->library.engine = engine_library;
    offsets->library.tier0 = tier0_library;
    offsets->library.input = input_library;
    offsets->library.sdl = sdl_library;

    const u64 resource_interface = get_interface(
        process, offsets->library.engine, "GameResourceServiceClientV0");
    if (!resource_interface) {
        return false;
    }
    offsets->interface.resource = resource_interface;

    offsets->interface.entity =
        read_u64(process, offsets->interface.resource + 0x50);
    offsets->interface.player = offsets->interface.entity + 0x10;

    offsets->interface.convar =
        get_interface(process, offsets->library.tier0, "VEngineCvar0");
    offsets->interface.input =
        get_interface(process, offsets->library.input, "InputSystemVersion0");

    // some inexplicable black magic
    offsets->direct.button_state = read_u32(
        process,
        get_interface_function(process, offsets->interface.input, 19) + 0x14);

    const u64 local_controller = scan_pattern(
        process, offsets->library.client,
        "\x48\x83\x3D\x00\x00\x00\x00\x00\x0F\x95\xC0\xC3", "xxx????xxxxx", 12);
    if (!local_controller) {
        return false;
    }
    offsets->direct.local_controller =
        get_relative_address(process, local_controller, 0x03, 0x07);

    const u64 view_matrix = scan_pattern(
        process, offsets->library.client,
        "\x48\x8D\x05\x00\x00\x00\x00\x4C\x8D\x05\x00\x00\x00\x00\x48\x8D\x0D",
        "xxx????xxx????xxx", 17);
    if (!view_matrix) {
        return false;
    }
    offsets->direct.view_matrix =
        get_relative_address(process, view_matrix + 0x07, 0x03, 0x07);

    offsets->convars.sensitivity =
        get_convar(process, offsets->interface.convar, "sensitivity");
    offsets->convars.ffa = get_convar(process, offsets->interface.convar,
                                      "mp_teammates_are_enemies");

    // dump netvars
    const u8 *client_dump = dump_library(process, offsets->library.client);
    if (!client_dump) {
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
                const char *name =
                    (char *)(ne_name_pointer - base + client_dump);
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
        if (!strncmp(name, "m_hPawn", 7)) {
            if (!network_enable || offsets->controller.pawn) {
                continue;
            }
            offsets->controller.pawn = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_iHealth", 9)) {
            if (!network_enable || offsets->pawn.health) {
                continue;
            }
            offsets->pawn.health = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_iTeamNum", 10)) {
            if (!network_enable || offsets->pawn.team) {
                continue;
            }
            offsets->pawn.team = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_lifeState", 11)) {
            if (!network_enable || offsets->pawn.life_state) {
                continue;
            }
            offsets->pawn.life_state = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_pClippingWeapon", 17)) {
            if (offsets->pawn.weapon) {
                continue;
            }
            offsets->pawn.weapon = *(u32 *)(entry + 0x10);
        } else if (!strncmp(name, "m_flFOVSensitivityAdjust", 24)) {
            if (offsets->pawn.sensitivity_multiplier) {
                continue;
            }
            offsets->pawn.sensitivity_multiplier = *(u32 *)(entry + 0x08);
        } else if (!strncmp(name, "m_pGameSceneNode", 16)) {
            if (offsets->pawn.game_scene_node) {
                continue;
            }
            offsets->pawn.game_scene_node = *(u32 *)(entry + 0x10);
        } else if (!strncmp(name, "m_vecViewOffset", 15)) {
            if (!network_enable || offsets->pawn.eye_offset) {
                continue;
            }
            offsets->pawn.eye_offset = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_aimPunchCache", 15)) {
            if (!network_enable || offsets->pawn.aim_punch_cache) {
                continue;
            }
            offsets->pawn.aim_punch_cache = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_iShotsFired", 13)) {
            if (!network_enable || offsets->pawn.shots_fired) {
                continue;
            }
            offsets->pawn.shots_fired = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_bDormant", 11)) {
            if (offsets->game_scene_node.dormant) {
                continue;
            }
            offsets->game_scene_node.dormant = *(u32 *)(entry + 0x08);
        } else if (!strncmp(name, "m_vecAbsOrigin", 14)) {
            if (!network_enable || offsets->game_scene_node.origin) {
                continue;
            }
            offsets->game_scene_node.origin = *(u32 *)(entry + 0x08 + 0x10);
        } else if (!strncmp(name, "m_modelState", 12)) {
            if (offsets->game_scene_node.model_state) {
                continue;
            }
            offsets->game_scene_node.model_state = *(u32 *)(entry + 0x08);
        }

        if (all_offsets_found(offsets)) {
            break;
        }
    }

    return true;
}

void run(ProcessHandle *process) {}
