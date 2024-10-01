#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"
#include "features.h"
#include "memory.h"
#include "mouse.h"
#include "offsets.h"

int main(void) {
    if (!setup_mouse()) {
        return 0;
    }

    // uinput mouse has to be destroyed on some signals
    signal(SIGTERM, terminate_mouse);
    signal(SIGSEGV, terminate_mouse);
    signal(SIGINT, terminate_mouse);

    const i64 pid = get_pid(PROCESS_NAME);
    if (!pid) {
        printf("could not find process\n");
        return 0;
    }

    ProcessHandle process = {0};
    if (!open_process(pid, &process)) {
        printf("could not open process\n");
        return 0;
    }

    Offsets offsets = {0};
    if (!find_offsets(&process, &offsets)) {
        printf("could not find library offsets\n");
        return 0;
    }

    while (true) {
        run(&process);
    }

    close_mouse();
    return 0;
}
