#include "process.hpp"

#include <fcntl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "constants.hpp"
#include "unistd.h"

std::optional<u64> GetPid(std::string process_name) {
    for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto filename = entry.path().filename().string();
        const auto exe_path = "/proc/" + filename + "/exe";
        if (access(exe_path.c_str(), F_OK) != 0) {
            continue;
        }
        const auto exe_name = std::filesystem::read_symlink(exe_path).string();
        const auto pos = exe_name.rfind('/');
        // rfind returns npos on fail
        if (pos == std::string::npos) {
            continue;
        }

        const auto name = exe_name.substr(pos + 1);
        if (name == process_name) {
            return std::stoull(filename);
        }
    }

    return std::nullopt;
}

bool ValidatePid(u64 pid) { return access(("/proc/" + std::to_string(pid)).c_str(), F_OK) == 0; }

std::optional<Process> OpenProcess(u64 pid) {
    if (!ValidatePid(pid)) {
        return std::nullopt;
    }
    const auto path = "/proc/" + std::to_string(pid) + "/mem";
    int memory = open(path.c_str(), O_RDONLY);
    if (memory == -1) {
        return std::nullopt;
    }
    return Process{.pid = pid, .memory = memory};
}

void CloseProcess(Process *process) { close(process->memory); }

u8 ReadU8(const Process *process, const u64 address) {
    u8 value = 0;
    pread(process->memory, &value, sizeof(u8), address);
    return value;
}

u16 ReadU16(const Process *process, const u64 address) {
    u16 value = 0;
    pread(process->memory, &value, sizeof(u16), address);
    return value;
}

u32 ReadU32(const Process *process, const u64 address) {
    u32 value = 0;
    pread(process->memory, &value, sizeof(u32), address);
    return value;
}

u64 ReadU64(const Process *process, const u64 address) {
    u64 value = 0;
    pread(process->memory, &value, sizeof(u64), address);
    return value;
}

i32 ReadI32(const Process *process, const u64 address) {
    i32 value = 0;
    pread(process->memory, &value, sizeof(i32), address);
    return value;
}

f32 ReadF32(const Process *process, const u64 address) {
    f32 value = 0.0;
    pread(process->memory, &value, sizeof(f32), address);
    return value;
}

glm::vec2 ReadVec2(const Process *process, const u64 address) {
    glm::vec2 value(0.0);
    pread(process->memory, &value, sizeof(glm::vec2), address);
    return value;
}

glm::vec3 ReadVec3(const Process *process, const u64 address) {
    glm::vec3 value(0.0);
    pread(process->memory, &value, sizeof(glm::vec3), address);
    return value;
}

glm::ivec4 ReadIVec4(const Process *process, const u64 address) {
    glm::ivec4 value(0);
    pread(process->memory, &value, sizeof(glm::ivec4), address);
    return value;
}

glm::mat4 ReadMat4(const Process *process, const u64 address) {
    glm::mat4 value(0.0);
    pread(process->memory, &value, sizeof(glm::mat4), address);
    return value;
}

std::string ReadString(const Process *process, const u64 address) {
    std::string value;
    for (u64 i = address; i < address + 512; i++) {
        u8 character = ReadU8(process, i);
        if (character == 0) {
            break;
        }
        value.push_back(character);
    }
    return value;
}

std::vector<u8> ReadBytes(const Process *process, const u64 address, const u64 count) {
    std::vector<u8> buffer(count);
    pread(process->memory, buffer.data(), count, address);
    return buffer;
}

std::optional<u64> GetModuleBaseAddress(const Process *process, const char *module_name) {
    std::ifstream maps("/proc/" + std::to_string(process->pid) + "/maps");
    std::string line;
    while (std::getline(maps, line)) {
        if (line.rfind(module_name) == std::string::npos) {
            continue;
        }
        const size_t index = line.find_first_of('-');
        const std::string address = line.substr(0, index);
        return std::stoull(address, nullptr, 16);
    }

    return std::nullopt;
}

u64 ModuleSize(const Process *process, const u64 module_address) {
    const u64 section_header_offset = ReadU64(process, module_address + ELF_SECTION_HEADER_OFFSET);
    const u64 section_header_entry_size =
        ReadU16(process, module_address + ELF_SECTION_HEADER_ENTRY_SIZE);
    const u64 section_header_num_entries =
        ReadU16(process, module_address + ELF_SECTION_HEADER_NUM_ENTRIES);

    return section_header_offset + section_header_entry_size * section_header_num_entries;
}

std::vector<u8> DumpModule(const Process *process, const u64 module_address) {
    const u64 module_size = ModuleSize(process, module_address);
    return ReadBytes(process, module_address, module_size);
}

std::optional<u64> ScanPattern(const Process *process, const std::vector<u8> pattern,
                               const std::vector<bool> mask, const u64 length,
                               const u64 module_address) {
    const auto module = DumpModule(process, module_address);
    if (module.size() < 500) {
        return std::nullopt;
    }

    for (u64 i = 0; i < module.size() - length; i++) {
        bool found = true;
        for (u64 j = 0; j < length; j++) {
            if (mask[j] && module[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return module_address + i;
        }
    }
    return std::nullopt;
}

u64 GetRelativeAddress(const Process *process, const u64 instruction, const u64 offset,
                       const u64 instruction_size) {
    const i32 rip_address = ReadI32(process, instruction + offset);
    return instruction + instruction_size + rip_address;
}

std::optional<u64> GetInterfaceOffset(const Process *process, const u64 module_address,
                                      const char *interface_name) {
    const std::optional<u64> create_interface_opt =
        GetModuleExport(process, module_address, "CreateInterface");
    if (!create_interface_opt.has_value()) {
        return std::nullopt;
    }
    const u64 create_interface = create_interface_opt.value();
    const std::optional<u64> export_address_opt =
        GetRelativeAddress(process, create_interface, 0x01, 0x05) + 0x10;
    if (!export_address_opt.has_value()) {
        return std::nullopt;
    }
    const u64 export_address = export_address_opt.value();

    u64 interface_entry =
        ReadU64(process, export_address + 0x07 + ReadU32(process, export_address + 0x03));

    while (true) {
        const u64 entry_name_address = ReadU64(process, interface_entry + 8);
        const std::string entry_name = ReadString(process, entry_name_address);
        if (entry_name.rfind(interface_name) != std::string::npos) {
            const u64 vfunc_address = ReadU64(process, interface_entry);
            return ReadU32(process, vfunc_address + 0x03) + vfunc_address + 0x07;
        }
        interface_entry = ReadU64(process, interface_entry + 0x10);
        if (interface_entry == 0) {
            break;
        }
    }

    return std::nullopt;
}

std::optional<u64> GetModuleExport(const Process *process, const u64 module_address,
                                   const char *export_name) {
    const u64 add = 0x18;

    const std::optional<u64> string_table_opt =
        GetAddressFromDynamicSection(process, module_address, 0x05);
    const std::optional<u64> symbol_table_opt =
        GetAddressFromDynamicSection(process, module_address, 0x06);
    if (!string_table_opt.has_value() || !symbol_table_opt.has_value()) {
        return std::nullopt;
    }
    const u64 string_table = string_table_opt.value();
    u64 symbol_table = symbol_table_opt.value();

    symbol_table += add;

    while (ReadU32(process, symbol_table) != 0) {
        const u64 st_name = ReadU32(process, symbol_table);
        const std::string name = ReadString(process, string_table + st_name);
        if (name == export_name) {
            return ReadU64(process, symbol_table + 0x08) + module_address;
        }
        symbol_table += add;
    }

    return std::nullopt;
}

std::optional<u64> GetAddressFromDynamicSection(const Process *process, const u64 module_address,
                                                const u64 tag) {
    const std::optional<u64> dynamic_section_offset_opt =
        GetSegmentFromPht(process, module_address, ELF_DYNAMIC_SECTION_PHT_TYPE);
    if (!dynamic_section_offset_opt.has_value()) {
        return std::nullopt;
    }
    const u64 dynamic_section_offset = dynamic_section_offset_opt.value();

    const u64 register_size = 8;
    u64 address = ReadU64(process, dynamic_section_offset + 2 * register_size) + module_address;

    while (true) {
        const u64 tag_address = address;
        const u64 tag_value = ReadU64(process, tag_address);

        if (tag_value == 0) {
            break;
        }
        if (tag_value == tag) {
            return ReadU64(process, tag_address + register_size);
        }

        address += register_size * 2;
    }

    return std::nullopt;
}

std::optional<u64> GetSegmentFromPht(const Process *process, const u64 module_address,
                                     const u64 tag) {
    const u64 first_entry =
        ReadU64(process, module_address + ELF_PROGRAM_HEADER_OFFSET) + module_address;
    const u64 entry_size = ReadU16(process, module_address + ELF_PROGRAM_HEADER_ENTRY_SIZE);

    for (u64 i = 0; i < ReadU16(process, module_address + ELF_PROGRAM_HEADER_NUM_ENTRIES); i++) {
        const u64 entry = first_entry + i * entry_size;
        if (ReadU32(process, entry) == tag) {
            return entry;
        }
    }

    return std::nullopt;
}

std::optional<u64> GetConvar(const Process *process, const u64 convar_offset,
                             const char *convar_name) {
    if (convar_offset == 0) {
        return std::nullopt;
    }

    const u64 objects = ReadU64(process, convar_offset + 64);
    for (u64 i = 0; i < ReadU64(process, convar_offset + 160); i++) {
        const u64 object = ReadU64(process, objects + i * 16);
        if (object == 0) {
            break;
        }

        const u64 name_address = ReadU64(process, object);
        const std::string name = ReadString(process, name_address);
        if (name == convar_name) {
            return object;
        }
    }

    return std::nullopt;
}

u64 GetInterfaceFunction(const Process *process, const u64 interface_address, const u64 index) {
    return ReadU64(process, ReadU64(process, interface_address) + (index * 8));
}
