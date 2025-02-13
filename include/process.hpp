#pragma once

#include <sys/uio.h>
#include <unistd.h>

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

#include "types.hpp"

extern bool file_mem;

class Process {
  public:
    i32 pid;
    i32 mem = -1;

    template <typename T>
    T Read(u64 address) {
        if (!file_mem) {
            T value;
            const struct iovec local_iov = {.iov_base = &value, .iov_len = sizeof(T)};
            const struct iovec remote_iov = {.iov_base = (void *)address, .iov_len = sizeof(T)};
            process_vm_readv(pid, &local_iov, 1, &remote_iov, 1, 0);
            return value;
        } else {
            T value;
            pread(mem, &value, sizeof(value), address);
            return value;
        }
    }

    template <typename T>
    void Write(u64 address, T value) {
        if (!file_mem) {
            const struct iovec local_iov = {.iov_base = &value, .iov_len = sizeof(T)};
            const struct iovec remote_iov = {.iov_base = (void *)address, .iov_len = sizeof(T)};
            process_vm_writev(pid, &local_iov, 1, &remote_iov, 1, 0);
        } else {
            pwrite(mem, &value, sizeof(value), address);
        }
    }

    std::string ReadString(u64 address);
    std::vector<u8> ReadBytes(u64 address, u64 count);

    std::optional<u64> GetModuleBaseAddress(const char *module_name);
    u64 ModuleSize(u64 module_address);
    std::vector<u8> DumpModule(u64 module_address);
    std::optional<u64> ScanPattern(
        std::vector<u8> pattern, std::vector<bool> mask, u64 length, u64 module_address);
    u64 GetRelativeAddress(u64 instruction, u64 offset, u64 instrution_size);
    std::optional<u64> GetInterfaceOffset(u64 module_address, const char *interface_name);
    std::optional<u64> GetModuleExport(u64 module_address, const char *export_name);
    std::optional<u64> GetAddressFromDynamicSection(u64 module_address, u64 tag);
    std::optional<u64> GetSegmentFromPht(u64 module_address, const u64 tag);
    std::optional<u64> GetConvar(u64 convar_offset, const char *convar_name);
    u64 GetInterfaceFunction(u64 interface_address, u64 index);
};

std::optional<i32> GetPid(std::string process_name);
bool ValidatePid(i32 pid);
std::optional<Process> OpenProcess(i32 pid);
