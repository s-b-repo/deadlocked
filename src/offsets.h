#ifndef OFFSETS_H
#define OFFSETS_H

#include "types.h"

struct LibraryOffsets {
    u64 client;
};

struct GeneralOffsets {
    u64 entity_list;
};

typedef struct Offsets {
    struct LibraryOffsets library;
    struct GeneralOffsets general;
} Offsets;

#endif
