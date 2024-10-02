#ifndef OFFSETS_H
#define OFFSETS_H

#include "types.h"

typedef struct LibraryOffsets {
    u64 client;
} LibraryOffsets;

typedef struct GeneralOffsets {
    u64 entity_list;
    u64 sdl_window;
} GeneralOffsets;

typedef struct InterfaceOffsets {
    u64 convar;
} InterfaceOffsets;

typedef struct DirectOffsets {
    u64 local_controller;
    u64 view_matrix;
    u64 button_state;
} DirectOffsets;

typedef struct ConvarOffsets {
    u64 sensitivity;
    u64 teammates_are_enemies;
} ConvarOffsets;

// todo: finish these offsets
typedef struct ControllerOffsets {
    u64 pawn;
} ControllerOffsets;

typedef struct PawnOffsets {
    u64 game_scene_node;
} PawnOffsets;

typedef struct GameSceneNodeOffsets {
    u64 dormant;
} GameSceneNodeOffsets;

typedef struct Offsets {
    LibraryOffsets library;
    GeneralOffsets general;
    InterfaceOffsets interface;
    DirectOffsets direct;

    ControllerOffsets controller;
    PawnOffsets pawn;
    GameSceneNodeOffsets game_scene_node;
} Offsets;

#endif
