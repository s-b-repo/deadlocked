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

u8 *dump_library(const ProcessHandle *process, const u64 address) {
    const u64 sh_offset = read_u64(process, address + ELF_SH_OFFSET);
    const u16 sh_entry_size = read_u16(process, address + ELF_SH_ENTRY_SIZE);
    const u16 sh_num_entries = read_u16(process, address + ELF_SH_NUM_ENTRIES);

    // section header table is the last thing in an elf file
    const u64 module_size = sh_offset + sh_entry_size * sh_num_entries;

    // allocate module size, plus 8 bytes for length
    u8 *buffer = malloc(module_size + sizeof(u64)) + sizeof(u64);
    // set length as the first 8 bytes, but set pointer at elf start
    *((u64 *)buffer - 1) = module_size;

    pread(process->memory, buffer, module_size, address);
    return buffer;
}

void free_dump(const u8 *dump) { free((dump - sizeof(u64))); }

bool check_elf_header(const u8 *data) {
    return data[0] == 0x7f && data[1] == 'E' && data[2] == 'L' &&
           data[3] == 'F';
}

u64 get_relative_address(const ProcessHandle *process, u64 instruction,
                         const u64 instruction_size, const u64 offset) {
    const i32 rip_address = read_i32(process, instruction + offset);
    return (u64)(instruction + instruction_size + rip_address);
}

u64 scan_pattern(const ProcessHandle *process, const u64 address,
                 const u8 *pattern, const u8 *mask, u64 length) {
    const u8 *dump = dump_library(process, address);

    for (u64 i = 0; i < (u64)(dump - sizeof(u64)); i++) {
        bool found = true;
        for (u64 j = 0; j < length; j++) {
            if (mask[j] == 'x' && dump[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            free_dump(dump);
            return address + i;
        }
    }

    free_dump(dump);
    return 0;
}

// segment is from program header, section is from section header
u64 get_segment(const ProcessHandle *process, const u64 address,
                const u32 tag) {
    const u64 first_entry =
        read_u64(process, address + ELF_PH_OFFSET) + address;
    const u16 entry_size = read_u16(process, address + ELF_PH_ENTRY_SIZE);

    for (size_t i = 0; i < read_u16(process, address + ELF_PH_NUM_ENTRIES);
         i++) {
        const u64 entry = first_entry + i * entry_size;
        if (read_u32(process, entry) == tag) {
            return entry;
        }
    }

    return 0;
}

#define REGISTER_SIZE 8
#define DYN_ENTRY_SIZE 16  // one Elf64_Dyn entry is 16 bytes long

u64 get_dynamic_export(const ProcessHandle *process, const u64 address,
                       const u32 tag) {
    const u64 dynamic_section =
        get_segment(process, address, ELF_DYNAMIC_SECTION_TAG);

    u64 current_entry =
        read_u64(process, dynamic_section + DYN_ENTRY_SIZE) + address;

    while (true) {
        const u64 entry = current_entry;
        const u64 tag_value = read_u64(process, entry);

        // DT_NULL ends dynamic table
        if (tag_value == 0) {
            break;
        }

        if (tag_value == tag) {
            return read_u64(process, entry + REGISTER_SIZE);
        }

        current_entry += DYN_ENTRY_SIZE;
    }

    return 0;
}

u64 get_library_export(const ProcessHandle *process, const u64 address,
                       const char *export_name) {
    // Elf64_Sym size (why 24 and not 16?)
    const u64 add = 24;

    // DT_STRTAB
    const u64 string_table = get_dynamic_export(process, address, 5);
    // DT_SYMTAB
    u64 symbol_table = get_dynamic_export(process, address, 6);

    symbol_table += add;
    while (read_u32(process, symbol_table) != 0) {
        // index into string table
        const u32 name_index = read_u32(process, symbol_table);
        const char *name = read_string(process, string_table + name_index);

        if (strcmp(name, export_name) == 0) {
            // st_value is 8 bytes from struct start
            return read_u64(process, symbol_table + 8) + address;
        }

        symbol_table += add;
    }

    return 0;
}

// done: fix
u64 get_interface(const ProcessHandle *process, const u64 address,
                  const char *interface_name) {
    const u64 interface_export =
        get_library_export(process, address, "CreateInterface");
    const u64 export_address =
        get_relative_address(process, interface_export, 0x05, 0x01) + 0x10;
    u64 interface_entry =
        read_u64(process, export_address + 0x07 +
                              read_u32(process, export_address + 0x03));

    const size_t interface_name_length = strlen(interface_name);
    while (true) {
        const u64 interface_name_address =
            read_u64(process, interface_entry + 8);
        const char *name = read_string(process, interface_name_address);

        if (strncmp(name, interface_name, interface_name_length) == 0) {
            const u64 vfunc_address = read_u64(process, interface_entry);
            return read_u32(process, vfunc_address + 0x03) + vfunc_address +
                   0x07;
        }

        interface_entry = read_u64(process, interface_entry + 0x10);
        if (interface_entry == 0) {
            break;
        }
    }

    return 0;
}

u64 get_convar(const ProcessHandle *process, u64 convar_offset,
               const char *convar_name) {
    const u64 objects = read_u64(process, convar_offset + 64);

    for (size_t i = 0; i < read_u32(process, convar_offset + 160); i++) {
        const u64 object = read_u64(process, objects + i * 16);
        if (object == 0) {
            break;
        }

        const char *name = read_string(process, read_u64(process, object));

        if (strcmp(convar_name, name) == 0) {
            return object;
        }
    }

    return 0;
}
