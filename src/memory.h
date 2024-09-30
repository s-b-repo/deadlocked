#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

typedef struct ProcessHandle {
    i64 pid;
    // posix open file descriptor
    int memory;
} ProcessHandle;

i64 get_pid(const char *process_name);
bool validate_pid(const i64 pid);
bool open_process(const i64 pid, ProcessHandle *process);

i8 read_i8(const ProcessHandle *process, const u64 address);
i16 read_i16(const ProcessHandle *process, const u64 address);
i32 read_i32(const ProcessHandle *process, const u64 address);
i64 read_i64(const ProcessHandle *process, const u64 address);

u8 read_u8(const ProcessHandle *process, const u64 address);
u16 read_u16(const ProcessHandle *process, const u64 address);
u32 read_u32(const ProcessHandle *process, const u64 address);
u64 read_u64(const ProcessHandle *process, const u64 address);

f32 read_f32(const ProcessHandle *process, const u64 address);
f64 read_f64(const ProcessHandle *process, const u64 address);

u8 *read_bytes(const ProcessHandle *process, const u64 address,
               const u64 count);
char *read_string(const ProcessHandle *process, const u64 address);

void write_u8(const ProcessHandle *process, const u64 address, u8 value);

u64 get_library_base_offset(const ProcessHandle *process,
                            const char *library_name);

#endif