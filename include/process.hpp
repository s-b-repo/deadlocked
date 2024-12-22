#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

#include "cs2/offsets.hpp"
#include "types.hpp"

struct Process {
    u64 pid;
    int memory;
};

u8 ReadU8(const Process *process, const u64 address);
u16 ReadU16(const Process *process, const u64 address);
u32 ReadU32(const Process *process, const u64 address);
u64 ReadU64(const Process *process, const u64 address);
i32 ReadI32(const Process *process, const u64 address);
f32 ReadF32(const Process *process, const u64 address);
glm::vec2 ReadVec2(const Process *process, const u64 address);
glm::vec3 ReadVec3(const Process *process, const u64 address);
glm::ivec4 ReadIVec4(const Process *process, const u64 address);
glm::mat4 ReadMat4(const Process *process, const u64 address);
std::string ReadString(const Process *process, const u64 address);
std::vector<u8> ReadBytes(const Process *process, const u64 address, const u64 count);

std::optional<u64> GetModuleBaseAddress(const Process *process, const char *module_name);
u64 ModuleSize(const Process *process, const u64 module_address);
std::vector<u8> DumpModule(const Process *process, const u64 module_address);
std::optional<u64> ScanPattern(const Process *process, const std::vector<u8> pattern, const std::vector<bool> mask,
                               const u64 length, const u64 module_address);
u64 GetRelativeAddress(const Process *process, const u64 instruction, const u64 offset, const u64 instrution_size);
std::optional<u64> GetInterfaceOffset(const Process *process, const u64 module_address, const char *interface_name);
std::optional<u64> GetModuleExport(const Process *process, const u64 module_address, const char *export_name);
std::optional<u64> GetAddressFromDynamicSection(const Process *process, const u64 module_address, const u64 tag);
std::optional<u64> GetSegmentFromPht(const Process *process, const u64 module_address, const u64 tag);
std::optional<u64> GetConvar(const Process *process, const u64 convar_offset, const char *convar_name);
u64 GetInterfaceFunction(const Process *process, const u64 interface_address, const u64 index);

std::optional<u64> GetPid(std::string process_name);
bool ValidatePid(u64 pid);
std::optional<Process> OpenProcess(u64 pid);
