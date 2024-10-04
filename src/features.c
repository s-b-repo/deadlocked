#include "features.h"

#include <stdio.h>

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

    const u64 size = *((u64 *)client_dump - 1);

    return true;
}

void run(ProcessHandle *process) {}
