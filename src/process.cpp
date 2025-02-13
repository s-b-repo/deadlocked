#include "process.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "constants.hpp"
#include "log.hpp"

std::optional<i32> GetPid(std::string process_name) {
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
            return std::stoi(filename);
        }
    }

    return std::nullopt;
}

bool ValidatePid(i32 pid) { return access(("/proc/" + std::to_string(pid)).c_str(), F_OK) == 0; }

std::optional<Process> OpenProcess(i32 pid) {
    if (!ValidatePid(pid)) {
        return std::nullopt;
    }
    if (!file_mem) {
        return Process{.pid = pid};
    } else {
        return Process{
            .pid = pid, .mem = open(("/proc/" + std::to_string(pid) + "/mem").c_str(), O_RDWR)};
    }
}

std::string Process::ReadString(u64 address) {
    std::string value;
    value.reserve(64);
    for (u64 i = address; i < address + 512; i += sizeof(u64)) {
        u64 chunk = Read<u64>(i);

        // https://graphics.stanford.edu/~seander/bithacks.html
        if (((chunk - 0x0101010101010101ULL) & ~chunk & 0x8080808080808080ULL) != 0) {
            // at least one byte is null, process each individually
            // Process each byte individually.
            for (int offset = 0; offset < 8; ++offset) {
                u8 byte = (chunk >> (offset * 8)) & 0xFF;
                if (byte == 0) return value;
                value.push_back(byte);
            }
        } else {
            // no null, just append the chunk
            for (int offset = 0; offset < 8; ++offset) {
                u8 byte = (chunk >> (offset * 8)) & 0xFF;
                value.push_back(byte);
            }
        }
    }
    return value;
}

std::vector<u8> Process::ReadBytes(u64 address, u64 count) {
    const auto path = "/proc/" + std::to_string(pid) + "/mem";
    i32 file = open(path.c_str(), O_RDONLY);
    std::vector<u8> buffer(count);
    pread(file, buffer.data(), count, address);
    close(file);
    return buffer;
}

std::optional<u64> Process::GetModuleBaseAddress(const char *module_name) {
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    std::string line;
    while (std::getline(maps, line)) {
        if (line.rfind(module_name) == std::string::npos) {
            continue;
        }
        const size_t index = line.find_first_of('-');
        const std::string address_str = line.substr(0, index);
        u64 address = std::stoull(address_str, nullptr, 16);
        if (address == 0) {
            Log(LogLevel::Warning, "address for module " + std::string(module_name) +
                                       " was 0, input string was \"" + line +
                                       "\", extracted address was " + address_str);
            continue;
        } else {
            Log(LogLevel::Debug,
                "module " + std::string(module_name) + " found at " + std::to_string(address));
            return address;
        }
    }

    Log(LogLevel::Warning, "could not find address for module " + std::string(module_name));
    return std::nullopt;
}

u64 Process::ModuleSize(u64 module_address) {
    const u64 section_header_offset = Read<u64>(module_address + ELF_SECTION_HEADER_OFFSET);
    const u64 section_header_entry_size = Read<u16>(module_address + ELF_SECTION_HEADER_ENTRY_SIZE);
    const u64 section_header_num_entries =
        Read<u16>(module_address + ELF_SECTION_HEADER_NUM_ENTRIES);

    return section_header_offset + section_header_entry_size * section_header_num_entries;
}

std::vector<u8> Process::DumpModule(u64 module_address) {
    const u64 module_size = ModuleSize(module_address);
    // should be 1 gb
    if (module_size == 0 || module_size > 1000000000) {
        Log(LogLevel::Error, "could not dump module at " + std::to_string(module_address));
        return std::vector<u8>();
    }
    return ReadBytes(module_address, module_size);
}

std::optional<u64> Process::ScanPattern(
    std::vector<u8> pattern, std::vector<bool> mask, u64 length, u64 module_address) {
    const auto module = DumpModule(module_address);
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

    Log(LogLevel::Warning, "broken signature: " + std::string(pattern.begin(), pattern.end()));
    return std::nullopt;
}

u64 Process::GetRelativeAddress(u64 instruction, u64 offset, u64 instruction_size) {
    const i32 rip_address = Read<i32>(instruction + offset);
    return instruction + instruction_size + rip_address;
}

std::optional<u64> Process::GetInterfaceOffset(u64 module_address, const char *interface_name) {
    const std::optional<u64> create_interface_opt =
        GetModuleExport(module_address, "CreateInterface");
    if (!create_interface_opt.has_value()) {
        Log(LogLevel::Error, "could not find CreateInterface export");
        return std::nullopt;
    }
    const u64 create_interface = create_interface_opt.value();
    const std::optional<u64> export_address_opt =
        GetRelativeAddress(create_interface, 0x01, 0x05) + 0x10;
    if (!export_address_opt.has_value()) {
        return std::nullopt;
    }
    const u64 export_address = export_address_opt.value();

    u64 interface_entry = Read<u64>(export_address + 0x07 + Read<u32>(export_address + 0x03));

    while (true) {
        const u64 entry_name_address = Read<u64>(interface_entry + 8);
        const std::string entry_name = ReadString(entry_name_address);
        if (entry_name.rfind(interface_name) != std::string::npos) {
            const u64 vfunc_address = Read<u64>(interface_entry);
            return Read<u32>(vfunc_address + 0x03) + vfunc_address + 0x07;
        }
        interface_entry = Read<u64>(interface_entry + 0x10);
        if (interface_entry == 0) {
            break;
        }
    }

    Log(LogLevel::Warning, "could not find interface offset for " + std::string(interface_name));
    return std::nullopt;
}

std::optional<u64> Process::GetModuleExport(u64 module_address, const char *export_name) {
    const u64 add = 0x18;

    const std::optional<u64> string_table_opt = GetAddressFromDynamicSection(module_address, 0x05);
    const std::optional<u64> symbol_table_opt = GetAddressFromDynamicSection(module_address, 0x06);
    if (!string_table_opt.has_value() || !symbol_table_opt.has_value()) {
        return std::nullopt;
    }
    const u64 string_table = string_table_opt.value();
    u64 symbol_table = symbol_table_opt.value();

    symbol_table += add;

    while (Read<u32>(symbol_table) != 0) {
        const u64 st_name = Read<u32>(symbol_table);
        const std::string name = ReadString(string_table + st_name);
        if (name == export_name) {
            return Read<u64>(symbol_table + 0x08) + module_address;
        }
        symbol_table += add;
    }

    Log(LogLevel::Warning, "could not find export " + std::string(export_name) + " in module at " +
                               std::to_string(module_address));
    return std::nullopt;
}

std::optional<u64> Process::GetAddressFromDynamicSection(u64 module_address, u64 tag) {
    const std::optional<u64> dynamic_section_offset_opt =
        GetSegmentFromPht(module_address, ELF_DYNAMIC_SECTION_PHT_TYPE);
    if (!dynamic_section_offset_opt.has_value()) {
        Log(LogLevel::Error, "could not find dynamic section in loaded elf");
        return std::nullopt;
    }
    const u64 dynamic_section_offset = dynamic_section_offset_opt.value();

    const u64 register_size = 8;
    u64 address = Read<u64>(dynamic_section_offset + 2 * register_size) + module_address;

    while (true) {
        const u64 tag_address = address;
        const u64 tag_value = Read<u64>(tag_address);

        if (tag_value == 0) {
            break;
        }
        if (tag_value == tag) {
            return Read<u64>(tag_address + register_size);
        }

        address += register_size * 2;
    }

    Log(LogLevel::Warning, "could not find tag " + std::to_string(tag) + " in dynamic section");
    return std::nullopt;
}

std::optional<u64> Process::GetSegmentFromPht(u64 module_address, u64 tag) {
    const u64 first_entry = Read<u64>(module_address + ELF_PROGRAM_HEADER_OFFSET) + module_address;
    const u64 entry_size = Read<u16>(module_address + ELF_PROGRAM_HEADER_ENTRY_SIZE);

    for (u64 i = 0; i < Read<u16>(module_address + ELF_PROGRAM_HEADER_NUM_ENTRIES); i++) {
        const u64 entry = first_entry + i * entry_size;
        if (Read<u32>(entry) == tag) {
            return entry;
        }
    }

    Log(LogLevel::Error, "could not find tag " + std::to_string(tag) +
                             " in program header table at " + std::to_string(module_address));
    return std::nullopt;
}

std::optional<u64> Process::GetConvar(u64 convar_offset, const char *convar_name) {
    if (convar_offset == 0) {
        return std::nullopt;
    }

    const u64 objects = Read<u64>(convar_offset + 64);
    for (u64 i = 0; i < Read<u64>(convar_offset + 160); i++) {
        const u64 object = Read<u64>(objects + i * 16);
        if (object == 0) {
            break;
        }

        const u64 name_address = Read<u64>(object);
        const std::string name = ReadString(name_address);
        if (name == convar_name) {
            return object;
        }
    }

    Log(LogLevel::Warning, "could not find convar " + std::string(convar_name));
    return std::nullopt;
}

u64 Process::GetInterfaceFunction(u64 interface_address, u64 index) {
    return Read<u64>(Read<u64>(interface_address) + (index * 8));
}
