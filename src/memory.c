#include "memory.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"

i64 get_pid(const char *process_name) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        return 0;
    }
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        // . and .. are not valid directories
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // if this is not null-initialized, it will keep the data from last loop
        char exe_path[1024] = "/proc/";
        snprintf(exe_path, sizeof(exe_path), "/proc/%s/cmdline", entry->d_name);

        if (access(exe_path, F_OK) != 0) {
            continue;
        }

        // reads from /proc/{pid}/cmdline, because wine has it as a command line
        // argument
        char exe_name[1024] = {0};
        FILE *cmdline = fopen(exe_path, "r");
        fgets(exe_name, sizeof(exe_name), cmdline);
        fclose(cmdline);

        // this has to be backslash, because wine and windows shit idk
        char *last_slash = strrchr(exe_name, '\\');
        if (last_slash == NULL) {
            continue;
        }
        last_slash += 1;
        if (strcmp(last_slash, process_name) == 0) {
            return strtoll(entry->d_name, NULL, 10);
        }
    }
    return 0;
}

bool validate_pid(const i64 pid) {
    char path[64] = {0};
    snprintf(path, sizeof(path), "/proc/%ld", pid);
    return access(path, F_OK) == 0;
}

bool open_process(const i64 pid, ProcessHandle *process) {
    if (!validate_pid(pid)) {
        return false;
    }

    // would want this to be stack allocated,
    process->pid = pid;
    char mem_path[64] = {0};
    snprintf(mem_path, sizeof(mem_path), "/proc/%ld/mem", pid);
    process->memory = open(mem_path, O_RDONLY);

    return true;
}

i8 read_i8(const ProcessHandle *process, const u64 address) {
    i8 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

i16 read_i16(const ProcessHandle *process, const u64 address) {
    i16 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

i32 read_i32(const ProcessHandle *process, const u64 address) {
    i32 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

i64 read_i64(const ProcessHandle *process, const u64 address) {
    i64 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

u8 read_u8(const ProcessHandle *process, const u64 address) {
    u8 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

u16 read_u16(const ProcessHandle *process, const u64 address) {
    u16 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

u32 read_u32(const ProcessHandle *process, const u64 address) {
    u32 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

u64 read_u64(const ProcessHandle *process, const u64 address) {
    u64 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

f32 read_f32(const ProcessHandle *process, const u64 address) {
    f32 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

f64 read_f64(const ProcessHandle *process, const u64 address) {
    f64 value = 0;
    pread(process->memory, &value, sizeof(value), address);
    return value;
}

u8 *read_bytes(const ProcessHandle *process, const u64 address,
               const u64 count) {
    u8 *buffer = calloc(count, 1);
    pread(process->memory, &buffer, count, address);
    return buffer;
}

// done: validate that this works
char *read_string(const ProcessHandle *process, const u64 address) {
    u32 current_size = 8;
    char *buffer = calloc(current_size, 1);

    u64 address_mut = address;

    u8 c = 0;
    u32 i = 0;
    while ((c = read_u8(process, address_mut++)) != 0) {
        buffer[i++] = c;
        if (i >= current_size) {
            current_size *= 2;
            buffer = realloc(buffer, current_size);
        }
    }
    buffer[i] = '\0';
    return buffer;
}

u64 get_library_base_offset(const ProcessHandle *process,
                            const char *library_name) {
    char maps_path[1024] = {0};
    snprintf(maps_path, sizeof(maps_path), "/proc/%ld/maps", process->pid);

    FILE *maps = fopen(maps_path, "r");
    if (maps == NULL) {
        return 0;
    }

    char line[1024] = {0};
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, library_name)) {
            fclose(maps);
            return strtoull(line, NULL, 16);
        }
    }

    fclose(maps);
    return 0;
}
