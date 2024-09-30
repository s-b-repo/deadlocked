#include <stdio.h>

#include "constants.h"
#include "features.h"
#include "memory.h"
#include "offsets.h"

int main(void) {
    const i64 pid = get_pid(PROCESS_NAME);
    if (!pid) {
        printf("could not find process");
        return 0;
    }

    ProcessHandle process = {0};
    if (!open_process(pid, &process)) {
        printf("could not open process");
        return 0;
    }

    Offsets offsets = {0};
    if (!find_offsets(&process, &offsets)) {
        printf("could not find library offsets");
        return 0;
    }

    while (true) {
        run(&process);
    }

    return 0;
}
