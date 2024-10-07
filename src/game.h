#ifndef CS2_H
#define CS2_H

#include "memory.h"
#include "offsets.h"

bool find_offsets(const ProcessHandle *process, Offsets *offsets);

Vec2 get_view_angles(const ProcessHandle *process, const Offsets *offsets,
                     const u64 pawn);

u64 get_local_controller(const ProcessHandle *process, const Offsets *offsets);

u64 get_client_entity(const ProcessHandle *process, const Offsets *offsets,
                      const u64 index);

u64 get_pawn(const ProcessHandle *process, const Offsets *offsets,
             const u64 controller);

f32 get_sensitivity(const ProcessHandle *process, const Offsets *offsets);

bool is_ffa(const ProcessHandle *process, const Offsets *offsets);

bool is_button_down(const ProcessHandle *process, const Offsets *offsets,
                    const u64 button);

i32 get_health(const ProcessHandle *process, const Offsets *offsets,
               const u64 pawn);

u8 get_team(const ProcessHandle *process, const Offsets *offsets,
            const u64 pawn);

u8 get_life_state(const ProcessHandle *process, const Offsets *offsets,
                  const u64 pawn);

u64 get_gs_node(const ProcessHandle *process, const Offsets *offsets,
                const u64 pawn);

bool is_dormant(const ProcessHandle *process, const Offsets *offsets,
                const u64 pawn);

Vec3 get_position(const ProcessHandle *process, const Offsets *offsets,
                  const u64 pawn);

Vec3 get_eye_position(const ProcessHandle *process, const Offsets *offsets,
                      const u64 pawn);

Vec3 get_bone_position(const ProcessHandle *process, const Offsets *offsets,
                       const u64 pawn, const u64 bone_index);

i32 get_shots_fired(const ProcessHandle *process, const Offsets *offsets,
                    const u64 pawn);

f32 get_fov_multiplier(const ProcessHandle *process, const Offsets *offsets,
                       const u64 pawn);

Vec2 get_aim_punch(const ProcessHandle *process, const Offsets *offsets,
                   const u64 pawn);

char *get_weapon(ProcessHandle *process, const Offsets *offsets, u64 pawn);

#endif
