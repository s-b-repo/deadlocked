#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <unistd.h>

#include "constants.h"
#include "features.h"
#include "memory.h"
#include "offsets.h"
#include "serial.h"

int main(void) {
    setup_serial();
    move_mouse(100, 0);
    close_serial();

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

    return 0;
}
