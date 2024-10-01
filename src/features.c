#include "features.h"

#include <stdio.h>

#include "config.h"
#include "constants.h"

bool find_offsets(ProcessHandle *process, Offsets *offsets) {
    const u64 client_library = get_library_base_offset(process, PROCESS_NAME);
    if (!client_library) {
        return false;
    }
    offsets->library.client = client_library;

    const u64 entity_list = scan_pattern(
        process, offsets->library.client,
        "\x48\x8B\x0D\x00\x00\x00\x00\x8B\xC5\x48\xC1\xE8", "xxx????xxxxx", 12);

    return true;
}

void run(ProcessHandle *process) {}
