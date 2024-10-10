#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "constants.h"
#include "features.h"
#include "game.h"
#include "memory.h"
#include "mouse.h"
#include "offsets.h"

extern Config config;

void loop(void) {
    const i64 pid = get_pid(PROCESS_NAME);
    if (!pid) {
        // printf("could not find process\n");
        return;
    }
    printf("game found, pid: %ld\n", pid);

    ProcessHandle process = {0};
    if (!open_process(pid, &process)) {
        printf("could not open process\n");
        return;
    }
    printf("opened process\n");

    Offsets offsets = {0};
    if (!find_offsets(&process, &offsets)) {
        printf("could not find offsets\n");
        return;
    }
    printf("offsets found\n");

    // more than 10ms is laggy, less borks the bot somehow
    const struct timespec sleep_time = {.tv_sec = 0, .tv_nsec = 10 * 1000000};
    while (true) {
        if (!validate_pid(pid)) {
            printf("game closed\n");
            return;
        }
        run(&process, &offsets);
        nanosleep(&sleep_time, NULL);
    }
}

int main(void) {
    if (!setup_mouse()) {
        return 0;
    }

    // uinput mouse has to be destroyed on some signals
    signal(SIGTERM, terminate_mouse);
    signal(SIGSEGV, terminate_mouse);
    signal(SIGINT, terminate_mouse);

    // parse config file, create if it does not exist
    if (access(CONFIG_FILE_NAME, F_OK) != 0) {
        FILE *file = fopen(CONFIG_FILE_NAME, "w");
        create_config(file);
        fclose(file);
        printf("config file generated, edit it to your liking, then restart\n");
        close_mouse();
    } else {
        FILE *file = fopen(CONFIG_FILE_NAME, "rw");
        parse_config(file);
        fclose(file);
    }

    const struct timespec sleep_time = {.tv_sec = 5, .tv_nsec = 0};
    while (true) {
        loop();
        nanosleep(&sleep_time, NULL);
    }

    close_mouse();
    return 0;
}
