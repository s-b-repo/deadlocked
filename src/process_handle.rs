use std::{fs::File, os::unix::fs::FileExt};

use glam::{Vec2, Vec3};

use crate::{
    constants::Constants, memory::{check_elf_header, read_u64_vec}, offsets::InterfaceOffsets
};

pub struct ProcessHandle {
    pub pid: u64,
    pub memory: File,
}

impl ProcessHandle {
    pub fn new(pid: u64, memory: File) -> Self {
        Self { pid, memory }
    }

    #[allow(unused)]
    pub fn read_i8(&self, address: u64) -> i8 {
        let mut buffer = [0; 1];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        i8::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_u8(&self, address: u64) -> u8 {
        let mut buffer = [0; 1];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        u8::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_i16(&self, address: u64) -> i16 {
        let mut buffer = [0; 2];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        i16::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_u16(&self, address: u64) -> u16 {
        let mut buffer = [0; 2];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        u16::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_i32(&self, address: u64) -> i32 {
        let mut buffer = [0; 4];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        i32::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_u32(&self, address: u64) -> u32 {
        let mut buffer = [0; 4];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        u32::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_i64(&self, address: u64) -> i64 {
        let mut buffer = [0; 8];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        i64::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_u64(&self, address: u64) -> u64 {
        let mut buffer = [0; 8];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        u64::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_f32(&self, address: u64) -> f32 {
        let mut buffer = [0; 4];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        f32::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_f64(&self, address: u64) -> f64 {
        let mut buffer = [0; 8];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        f64::from_ne_bytes(buffer)
    }

    #[allow(unused)]
    pub fn read_string(&self, address: u64) -> String {
        let mut string = String::new();
        let mut i = address;
        loop {
            let c = self.read_u8(i);
            if c == 0 {
                break;
            }
            string.push(c as char);
            i += 1;
        }
        string
    }

    #[allow(unused)]
    pub fn read_bytes(&self, address: u64, count: u64) -> Vec<u8> {
        let mut buffer = vec![0u8; count as usize];
        self.memory.read_at(&mut buffer, address).unwrap_or(0);
        buffer
    }

    #[allow(unused)]
    pub fn read_vec2(&self, address: u64) -> Vec2 {
        let x = self.read_f32(address);
        let y = self.read_f32(address + 0x04);
        Vec2::from_slice(&[x, y])
    }

    #[allow(unused)]
    pub fn read_vec3(&self, address: u64) -> Vec3 {
        let x = self.read_f32(address);
        let y = self.read_f32(address + 0x04);
        let z = self.read_f32(address + 0x08);
        Vec3::from_slice(&[x, y, z])
    }

    pub fn dump_module(&self, address: u64) -> Vec<u8> {
        let module_size = self.module_size(address);
        self.read_bytes(address, module_size)
    }

    pub fn scan_pattern(&self, pattern: &[u8], mask: &[u8], base_address: u64) -> Option<u64> {
        if pattern.len() != mask.len() {
            println!(
                "pattern is {} bytes, mask is {} bytes long",
                pattern.len(),
                mask.len()
            );
            return None;
        }

        let module = self.dump_module(base_address);
        if module.len() < 500 {
            return None;
        }

        let pattern_length = pattern.len();
        let stop_index = module.len() - pattern_length;
        'outer: for i in 0..stop_index {
            for j in 0..pattern_length {
                if mask[j] == b'x' && module[i + j] != pattern[j] {
                    continue 'outer;
                }
            }
            return Some(base_address + i as u64);
        }
        None
    }

    pub fn get_relative_address(
        &self,
        instruction: u64,
        offset: u64,
        instruction_size: u64,
    ) -> u64 {
        // rip is instruction pointer
        let rip_address = self.read_i32(instruction + offset);
        instruction
            .wrapping_add(instruction_size)
            .wrapping_add(rip_address as u64)
    }

    pub fn get_interface_offset(&self, base_address: u64, interface_name: &str) -> Option<u64> {
        let create_interface = self.get_module_export(base_address, "CreateInterface")?;
        let export_address = self.get_relative_address(create_interface, 0x01, 0x05) + 0x10;

        let mut interface_entry =
            self.read_u64(export_address + 0x07 + self.read_u32(export_address + 0x03) as u64);

        loop {
            let entry_name_address = self.read_u64(interface_entry + 8);
            let entry_name = self.read_string(entry_name_address);
            if entry_name.starts_with(interface_name) {
                let vfunc_address = self.read_u64(interface_entry);
                return Some(self.read_u32(vfunc_address + 0x03) as u64 + vfunc_address + 0x07);
            }
            interface_entry = self.read_u64(interface_entry + 0x10);
            if interface_entry == 0 {
                break;
            }
        }
        None
    }

    pub fn get_module_export(&self, base_address: u64, export_name: &str) -> Option<u64> {
        let module = self.dump_module(base_address);
        if !check_elf_header(module) {
            return None;
        }

        let add = 0x18;
        let length = 0x08;

        let string_table = self.get_address_from_dynamic_section(base_address, 0x05)?;
        let mut symbol_table = self.get_address_from_dynamic_section(base_address, 0x06)?;

        symbol_table += add;

        while self.read_u32(symbol_table) != 0 {
            let st_name = self.read_u32(symbol_table);
            let name = self.read_string(string_table + st_name as u64);
            if name == export_name {
                let address_vec = self.read_bytes(symbol_table + length, length);
                return Some(read_u64_vec(&address_vec, 0) + base_address);
            }
            symbol_table += add;
        }
        None
    }

    pub fn get_address_from_dynamic_section(&self, base_address: u64, tag: u64) -> Option<u64> {
        let dynamic_section_offset =
            self.get_segment_from_pht(base_address, Constants::ELF_DYNAMIC_SECTION_PHT_TYPE)?;

        let register_size = 8;
        let mut address = self.read_u64(dynamic_section_offset + 2 * register_size) + base_address;

        loop {
            let tag_address = address;
            let tag_value = self.read_u64(tag_address);

            if tag_value == 0 {
                break;
            }
            if tag_value == tag {
                return Some(self.read_u64(tag_address + register_size));
            }

            address += register_size * 2;
        }
        None
    }

    pub fn get_segment_from_pht(&self, base_address: u64, tag: u64) -> Option<u64> {
        let first_entry =
            self.read_u64(base_address + Constants::ELF_PROGRAM_HEADER_OFFSET) + base_address;
        let entry_size = self.read_u16(base_address + Constants::ELF_PROGRAM_HEADER_ENTRY_SIZE) as u64;

        for i in 0..self.read_u16(base_address + Constants::ELF_PROGRAM_HEADER_NUM_ENTRIES) {
            let entry = first_entry + i as u64 * entry_size;
            if self.read_u32(entry) as u64 == tag {
                return Some(entry);
            }
        }
        None
    }

    pub fn get_convar(&self, offsets: &InterfaceOffsets, convar_name: &str) -> Option<u64> {
        if offsets.cvar == 0 {
            return None;
        }

        let objects = self.read_u64(offsets.cvar + 64);
        for i in 0..self.read_u32(offsets.cvar + 160) as u64 {
            let object = self.read_u64(objects + i * 16);
            if object == 0 {
                break;
            }

            let name_address = self.read_u64(object);
            let name = self.read_string(name_address);
            if name == convar_name {
                return Some(object);
            }
        }
        None
    }

    pub fn module_size(&self, base_address: u64) -> u64 {
        let section_header_offset = self.read_u64(base_address + Constants::ELF_SECTION_HEADER_OFFSET);
        let section_header_entry_size =
            self.read_u16(base_address + Constants::ELF_SECTION_HEADER_ENTRY_SIZE);
        let section_header_num_entries =
            self.read_u16(base_address + Constants::ELF_SECTION_HEADER_NUM_ENTRIES);

        section_header_offset + section_header_entry_size as u64 * section_header_num_entries as u64
    }

    pub fn get_interface_function(&self, interface_address: u64, index: u64) -> u64 {
        self.read_u64(self.read_u64(interface_address) + (index * 8))
    }
}
