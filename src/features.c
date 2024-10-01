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

    //const u64 entity_list =

    return true;
}

void run(ProcessHandle *process) {
}
