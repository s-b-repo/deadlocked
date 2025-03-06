#include "process.hpp"

#include <fcntl.h>
#include <unistd.h>
#ifdef __AVX2__
#include <immintrin.h>
#endif

#include <filesystem>
#include <fstream>
#include <string>

#include "constants.hpp"
#include "log.hpp"

std::optional<i32> GetPid(const std::string &process_name) {
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

bool ValidatePid(const i32 pid) {
    return access(("/proc/" + std::to_string(pid)).c_str(), F_OK) == 0;
}

std::optional<Process> OpenProcess(const i32 pid) {
    if (!ValidatePid(pid)) {
        return std::nullopt;
    }
    if (!flags.file_mem) {
        return Process {.pid = pid};
    }
    return Process {
        .pid = pid, .mem = open(("/proc/" + std::to_string(pid) + "/mem").c_str(), O_RDWR)};
}

std::string Process::ReadString(const u64 address) {
    std::string value;
    value.reserve(32);
#ifdef __AVX2__
    // 32 bytes at a time
    for (u64 i = address; i < address + 512; i += 32) {
        __m256i chunk = Read<__m256i>(i);

        // check if any byte is zero
        __m256i zeros = _mm256_setzero_si256();
        __m256i cmp = _mm256_cmpeq_epi8(chunk, zeros);
        i32 mask = _mm256_movemask_epi8(cmp);

        // store back into char buffer
        alignas(32) char block[32];
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(block), chunk);

        if (mask != 0) {
            // at least one byte is zero, append all bytes including zero
            u32 first_zero = __builtin_ctz(mask);
            value.append(block, first_zero);
            return value;
        } else {
            // no null
            value.append(block, 32);
        }
    }
#else
    // 8 bytes at a time
    for (u64 i = address; i < address + 512; i += sizeof(u64)) {
        const u64 chunk = Read<u64>(i);

        for (i32 offset = 0; offset < 8; ++offset) {
            const u8 byte = chunk >> offset * 8 & 0xFF;
            if (byte == 0) {
                return value;
            }
            value.push_back(static_cast<char>(byte));
        }
    }
#endif
    return value;
}

std::vector<u8> Process::ReadBytes(const u64 address, const u64 count) const {
    const auto path = "/proc/" + std::to_string(pid) + "/mem";
    const i32 file = open(path.c_str(), O_RDONLY);
    std::vector<u8> buffer(count);
    pread(file, buffer.data(), count, static_cast<long>(address));
    close(file);
    return buffer;
}

std::optional<u64> Process::GetModuleBaseAddress(const std::string &module_name) const {
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
            Log(LogLevel::Warning, "address for module " + module_name +
                                       " was 0, in put string was \"" + line +
                                       "\", "
                                       "extracted address was " +
                                       address_str);
            continue;
        }

        return address;
    }

    Log(LogLevel::Warning, "could not find address for module " + std::string(module_name));
    return std::nullopt;
}

u64 Process::ModuleSize(const u64 module_address) {
    const u64 section_header_offset = Read<u64>(module_address + ELF_SECTION_HEADER_OFFSET);
    const u64 section_header_entry_size = Read<u16>(module_address + ELF_SECTION_HEADER_ENTRY_SIZE);
    const u64 section_header_num_entries =
        Read<u16>(module_address + ELF_SECTION_HEADER_NUM_ENTRIES);

    return section_header_offset + section_header_entry_size * section_header_num_entries;
}

std::vector<u8> Process::DumpModule(const u64 module_address) {
    const u64 module_size = ModuleSize(module_address);
    // should be 1 gb
    if (module_size == 0 || module_size > 1000000000) {
        Log(LogLevel::Error, "could not dump module at " + std::to_string(module_address));
        return {};
    }
    return ReadBytes(module_address, module_size);
}

std::optional<u64> Process::ScanPattern(
    const std::vector<u8> &pattern, const std::vector<bool> &mask, const u64 length,
    const u64 module_address) {
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

u64 Process::GetRelativeAddress(
    const u64 instruction, const u64 offset, const u64 instruction_size) {
    const i32 rip_address = Read<i32>(instruction + offset);
    return instruction + instruction_size + rip_address;
}

std::optional<u64> Process::GetInterfaceOffset(
    const u64 module_address, const std::string &interface_name) {
    const auto create_interface = GetModuleExport(module_address, "CreateInterface");
    if (!create_interface) {
        Log(LogLevel::Error, "could not find CreateInterface export");
        return std::nullopt;
    }

    const u64 export_address = GetRelativeAddress(*create_interface, 0x01, 0x05) + 0x10;
    if (!export_address) {
        return std::nullopt;
    }

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

std::optional<u64> Process::GetModuleExport(
    const u64 module_address, const std::string &export_name) {
    constexpr u64 add = 0x18;

    const std::optional<u64> string_table = GetAddressFromDynamicSection(module_address, 0x05);
    std::optional<u64> symbol_table = GetAddressFromDynamicSection(module_address, 0x06);
    if (!string_table || !symbol_table) {
        return std::nullopt;
    }

    *symbol_table += add;

    while (Read<u32>(*symbol_table) != 0) {
        const u64 st_name = Read<u32>(*symbol_table);
        const std::string name = ReadString(*string_table + st_name);
        if (name == export_name) {
            return Read<u64>(*symbol_table + 0x08) + module_address;
        }
        *symbol_table += add;
    }

    Log(LogLevel::Warning, "could not find export " + std::string(export_name) + " in module at " +
                               std::to_string(module_address));
    return std::nullopt;
}

std::optional<u64> Process::GetAddressFromDynamicSection(const u64 module_address, const u64 tag) {
    const std::optional<u64> dynamic_section_offset =
        GetSegmentFromPht(module_address, ELF_DYNAMIC_SECTION_PHT_TYPE);
    if (!dynamic_section_offset) {
        Log(LogLevel::Error, "could not find dynamic section in loaded elf");
        return std::nullopt;
    }

    constexpr u64 register_size = 8;
    u64 address = Read<u64>(*dynamic_section_offset + 2 * register_size) + module_address;

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

std::optional<u64> Process::GetSegmentFromPht(const u64 module_address, const u64 tag) {
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

std::optional<u64> Process::GetConvar(const u64 convar_offset, const std::string &convar_name) {
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

u64 Process::GetInterfaceFunction(const u64 interface_address, const u64 index) {
    return Read<u64>(Read<u64>(interface_address) + index * 8);
}
