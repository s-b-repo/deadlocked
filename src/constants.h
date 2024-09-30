#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PROCESS_NAME "project8.exe"

#define CLIENT_LIB "client.dll"

// elf program header offsets
#define ELF_PH_OFFSET 0x20
#define ELF_PH_ENTRY_SIZE 0x36
#define ELF_PH_NUM_ENTRIES 0x38

// elf section header offsets
#define ELF_SH_OFFSET 0x28
#define ELF_SH_ENTRY_SIZE 0x3A
#define ELF_SH_NUM_ENTRIES 0x3C
// section header string table index
#define ELF_SH_ST_INDEX 0x3E

#define ELF_DYNAMIC_SECTION_TAG 0x02

#endif