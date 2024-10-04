#ifndef OFFSETS_H
#define OFFSETS_H

#include "types.h"

typedef struct LibraryOffsets {
    u64 client;
    u64 engine;
    u64 tier0;
    u64 input;
    u64 sdl;
} LibraryOffsets;

typedef struct GeneralOffsets {
    u64 entity_list;
    u64 sdl_window;
} GeneralOffsets;

typedef struct InterfaceOffsets {
    u64 convar;
    u64 resource;
    u64 entity;
    u64 player;
    u64 input;
} InterfaceOffsets;

typedef struct DirectOffsets {
    u64 local_controller;
    u64 view_matrix;
    u64 button_state;
} DirectOffsets;

typedef struct ConvarOffsets {
    u64 sensitivity;
    u64 ffa;
} ConvarOffsets;

// todo: finish these offsets
typedef struct ControllerOffsets {
    u64 pawn;  // Pointer -> Pawn (m_hPawn)
} ControllerOffsets;

typedef struct PawnOffsets {
    u64 health;                  // i32 (m_iHealth)
    u64 team;                    // u8 (m_iTeamNum)
    u64 life_state;              // u8 (m_lifeState)
    u64 weapon;                  // Pointer -> Weapon (m_pClippingWeapon)
    u64 sensitivity_multiplier;  // f32 (m_flFOVSensitivityAdjust)
    u64 game_scene_node;         // Pointer -> GameSceneNode (m_pGameSceneNode)
    u64 eye_offset;              // vec3 (m_vecViewOffset)
    u64 aim_punch_cache;         // Vector<vec3> (m_aimPunchCache)
    u64 shots_fired;             // i32 (m_iShotsFired)
} PawnOffsets;

typedef struct GameSceneNodeOffsets {
    u64 dormant;      // bool (m_bDormant)
    u64 origin;       // vec3 (m_vecAbsOrigin)
    u64 model_state;  // Pointer -> ModelState (m_modelState)
} GameSceneNodeOffsets;

typedef struct Offsets {
    LibraryOffsets library;
    GeneralOffsets general;
    InterfaceOffsets interface;
    DirectOffsets direct;
    ConvarOffsets convars;

    ControllerOffsets controller;
    PawnOffsets pawn;
    GameSceneNodeOffsets game_scene_node;
} Offsets;

#endif
