use std::{sync::mpsc, thread::sleep, time::Instant};

use crate::{
    config::{AimbotConfig, LOOP_DURATION},
    constants::{CS2Constants, WEAPON_UNKNOWN},
    cs2::offsets::Offsets,
    key_codes::KeyCode,
    memory::{
        get_module_base_address, get_pid, open_process, read_string_vec, read_u32_vec,
        read_u64_vec, validate_pid,
    },
    message::{Game, Message},
    process_handle::ProcessHandle,
    target::Target,
    weapon_class::{self, WeaponClass},
};
mod bones;
pub mod offsets;

#[derive(Debug)]
pub struct CS2 {
    tx: mpsc::Sender<Message>,
    rx: mpsc::Receiver<Message>,
    config: AimbotConfig,
    offsets: Offsets,
    target: Target,
}

impl CS2 {
    pub fn new(tx: mpsc::Sender<Message>, rx: mpsc::Receiver<Message>) -> Self {
        Self {
            tx,
            rx,
            config: AimbotConfig::default(),
            offsets: Offsets::default(),
            target: Target::default(),
        }
    }

    pub fn run(&mut self) {
        loop {
            let pid = match get_pid(CS2Constants::PROCESS_NAME) {
                Some(pid) => pid,
                None => continue,
            };

            let process = match open_process(pid) {
                Some(process) => process,
                None => continue,
            };

            self.offsets = match self.find_offsets(&process) {
                Some(offsets) => offsets,
                None => continue,
            };

            loop {
                if !validate_pid(pid) {
                    break;
                }
                self.main_loop(&process);
            }
        }
    }

    fn main_loop(&mut self, process: &ProcessHandle) {
        let start = Instant::now();

        if let Ok(message) = self.rx.try_recv() {
            self.parse_message(message);
        }

        self.aimbot(process);

        let elapsed = start.elapsed();
        if elapsed < LOOP_DURATION {
            sleep(LOOP_DURATION - elapsed);
        } else {
            println!("loop exceeded max duration: took {}ms", elapsed.as_millis());
        }
    }

    fn parse_message(&mut self, message: Message) {
        match message {
            Message::ConfigEnabled(Game::CS2, enabled) => self.config.enabled = enabled,
            Message::ConfigHotkey(Game::CS2, hotkey) => self.config.hotkey = hotkey,
            Message::ConfigStartBullet(Game::CS2, start_bullet) => {
                self.config.start_bullet = start_bullet
            }
            Message::ConfigAimLock(Game::CS2, aim_lock) => self.config.aim_lock = aim_lock,
            Message::ConfigVisibilityCheck(Game::CS2, visibility_check) => {
                self.config.visibility_check = visibility_check
            }
            Message::ConfigFOV(Game::CS2, fov) => self.config.fov = fov,
            Message::ConfigSmooth(Game::CS2, smooth) => self.config.smooth = smooth,
            Message::ConfigMultibone(Game::CS2, multibone) => self.config.multibone = multibone,
            Message::ConfigPauseWhenSpectated(Game::CS2, pause_when_spectated) => {
                self.config.pause_when_spectated = pause_when_spectated
            }
            _ => {}
        }
    }

    fn aimbot(&mut self, process: &ProcessHandle) {
        let local_controller = self.get_local_controller(process);
        let local_pawn = match self.get_pawn(process, local_controller) {
            Some(pawn) => pawn,
            None => {
                self.reset();
                return;
            }
        };

        let team = self.get_team(process, local_pawn);
        if team != CS2Constants::TEAM_CT && team != CS2Constants::TEAM_T {
            self.reset();
            return;
        }

        let weapon_class = self.get_weapon_class(process, local_pawn);
        if [
            WeaponClass::Unknown,
            WeaponClass::Knife,
            WeaponClass::Grenade,
        ]
        .contains(&weapon_class)
        {
            self.reset();
            return;
        }

        let aimbot_active = self.is_button_down(process, &self.config.hotkey);
    }

    fn reset(&mut self) {
        self.target = Target::default();
    }

    fn find_offsets(&self, process: &ProcessHandle) -> Option<Offsets> {
        let mut offsets = Offsets::default();

        let client_address = get_module_base_address(process, CS2Constants::CLIENT_LIB);
        offsets.library.client = client_address?;

        let engine_address = get_module_base_address(process, CS2Constants::ENGINE_LIB);
        offsets.library.engine = engine_address?;

        let tier0_address = get_module_base_address(process, CS2Constants::TIER0_LIB);
        offsets.library.tier0 = tier0_address?;

        let input_address = get_module_base_address(process, CS2Constants::INPUT_LIB);
        offsets.library.input = input_address?;

        let sdl_address = get_module_base_address(process, CS2Constants::SDL_LIB);
        offsets.library.sdl = sdl_address?;

        let resource_offset =
            process.get_interface_offset(offsets.library.engine, "GameResourceServiceClientV0");
        offsets.interface.resource = resource_offset?;

        offsets.interface.entity = process.read_u64(offsets.interface.resource + 0x50);
        offsets.interface.player = offsets.interface.entity + 0x10;

        offsets.interface.cvar =
            process.get_interface_offset(offsets.library.tier0, "VEngineCvar0")?;
        offsets.interface.input =
            process.get_interface_offset(offsets.library.input, "InputSystemVersion0")?;

        // seems to be in .text section (executable instructions)
        let local_player = process.scan_pattern(
            &[
                0x48, 0x83, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x95, 0xC0, 0xC3,
            ],
            "xxx????xxxxx".as_bytes(),
            offsets.library.client,
        );
        offsets.direct.local_player = process.get_relative_address(local_player?, 0x03, 0x08);
        offsets.direct.button_state = process
            .read_u32(process.get_interface_function(offsets.interface.input, 19) + 0x14)
            as u64;

        offsets.convar.ffa = process.get_convar(&offsets.interface, "mp_teammates_are_enemies")?;
        offsets.convar.sensitivity = process.get_convar(&offsets.interface, "sensitivity")?;

        let client_module_size = process.module_size(offsets.library.client);
        let client_dump = process.dump_module(offsets.library.client);

        let base = offsets.library.client;
        for i in (0..=(client_module_size - 8)).rev().step_by(8) {
            let mut network_enable = false;

            let mut name_pointer = read_u64_vec(&client_dump, i);
            if name_pointer >= base && name_pointer <= base + client_module_size {
                name_pointer = read_u64_vec(&client_dump, name_pointer - base);
                if name_pointer >= base && name_pointer <= base + client_module_size {
                    let name = read_string_vec(&client_dump, name_pointer - base);
                    if name.to_lowercase() == "MNetworkEnable".to_lowercase() {
                        network_enable = true;
                    }
                }
            }

            let name_ptr = match network_enable {
                true => read_u64_vec(&client_dump, i + 0x08),
                false => read_u64_vec(&client_dump, i),
            };

            if name_ptr < base || name_ptr > base + client_module_size {
                continue;
            }

            let netvar_name = read_string_vec(&client_dump, name_ptr - base);

            match netvar_name.as_str() {
                "m_hPawn" => {
                    if !network_enable || offsets.controller.pawn != 0 {
                        continue;
                    }
                    offsets.controller.pawn = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_iHealth" => {
                    if !network_enable || offsets.pawn.health != 0 {
                        continue;
                    }
                    offsets.pawn.health = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_iTeamNum" => {
                    if !network_enable || offsets.pawn.team != 0 {
                        continue;
                    }
                    offsets.pawn.team = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_lifeState" => {
                    if !network_enable || offsets.pawn.life_state != 0 {
                        continue;
                    }
                    offsets.pawn.life_state = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_pClippingWeapon" => {
                    if offsets.pawn.weapon != 0 {
                        continue;
                    }
                    offsets.pawn.weapon = read_u32_vec(&client_dump, i + 0x10) as u64;
                }
                "m_flFOVSensitivityAdjust" => {
                    if offsets.pawn.fov_multiplier != 0 {
                        continue;
                    }
                    offsets.pawn.fov_multiplier = read_u32_vec(&client_dump, i + 0x08) as u64;
                }
                "m_pGameSceneNode" => {
                    if offsets.pawn.game_scene_node != 0 {
                        continue;
                    }
                    offsets.pawn.game_scene_node = read_u32_vec(&client_dump, i + 0x10) as u64;
                }
                "m_vecViewOffset" => {
                    if !network_enable || offsets.pawn.eye_offset != 0 {
                        continue;
                    }
                    offsets.pawn.eye_offset = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_aimPunchCache" => {
                    if !network_enable || offsets.pawn.aim_punch_cache != 0 {
                        continue;
                    }
                    offsets.pawn.aim_punch_cache =
                        read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_iShotsFired" => {
                    if !network_enable || offsets.pawn.shots_fired != 0 {
                        continue;
                    }
                    offsets.pawn.shots_fired = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "v_angle" => {
                    if offsets.pawn.view_angles != 0 {
                        continue;
                    }
                    offsets.pawn.view_angles = read_u32_vec(&client_dump, i + 0x08) as u64;
                }
                "m_entitySpottedState" => {
                    if !network_enable || offsets.pawn.spotted_state != 0 {
                        continue;
                    }
                    offsets.pawn.spotted_state = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_pObserverServices" => {
                    if offsets.pawn.observer_services != 0 {
                        continue;
                    }
                    offsets.pawn.observer_services = read_u32_vec(&client_dump, i + 0x08) as u64;
                }
                "m_bDormant" => {
                    if offsets.game_scene_node.dormant != 0 {
                        continue;
                    }
                    offsets.game_scene_node.dormant = read_u32_vec(&client_dump, i + 0x08) as u64;
                }
                "m_vecAbsOrigin" => {
                    if !network_enable || offsets.game_scene_node.origin != 0 {
                        continue;
                    }
                    offsets.game_scene_node.origin =
                        read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_modelState" => {
                    if offsets.game_scene_node.model_state != 0 {
                        continue;
                    }
                    offsets.game_scene_node.model_state =
                        read_u32_vec(&client_dump, i + 0x08) as u64;
                }
                "m_bSpotted" => {
                    if offsets.spotted_state.spotted != 0 {
                        continue;
                    }
                    offsets.spotted_state.spotted = read_u32_vec(&client_dump, i + 0x10) as u64;
                }
                "m_bSpottedByMask" => {
                    if !network_enable || offsets.spotted_state.mask != 0 {
                        continue;
                    }
                    offsets.spotted_state.mask = read_u32_vec(&client_dump, i + 0x08 + 0x10) as u64;
                }
                "m_hObserverTarget" => {
                    if offsets.observer_service.target != 0 {
                        continue;
                    }
                    offsets.observer_service.target = read_u32_vec(&client_dump, i + 0x08) as u64;
                }
                _ => {}
            }

            if offsets.all_found() {
                return Some(offsets);
            }
        }
        None
    }

    fn get_local_controller(&self, process: &ProcessHandle) -> u64 {
        process.read_u64(self.offsets.direct.local_player)
    }

    fn get_client_entity(&self, process: &ProcessHandle, index: u64) -> Option<u64> {
        // wtf is this doing, and how?
        let v1 = process.read_u64(self.offsets.interface.entity + 0x08 * (index >> 9) + 0x10);
        if v1 == 0 {
            return None;
        }
        // what?
        let entity = process.read_u64(v1 + 120 * (index & 0x1ff));
        if entity == 0 {
            return None;
        }
        Some(entity)
    }

    fn get_pawn(&self, process: &ProcessHandle, controller: u64) -> Option<u64> {
        let v1 = process.read_i32(controller + self.offsets.controller.pawn);
        if v1 == -1 {
            return None;
        }

        // what the fuck is this doing?
        let v2 = process.read_u64(self.offsets.interface.player + 8 * ((v1 as u64 & 0x7fff) >> 9));
        if v2 == 0 {
            return None;
        }

        // bit-fuckery, why is this needed exactly?
        let entity = process.read_u64(v2 + 120 * (v1 as u64 & 0x1ff));
        if entity == 0 {
            return None;
        }
        Some(entity)
    }

    fn get_health(&self, process: &ProcessHandle, pawn: u64) -> i32 {
        let health = process.read_i32(pawn + self.offsets.pawn.health);
        if !(0..=100).contains(&health) {
            return 0;
        }
        health
    }

    fn get_team(&self, process: &ProcessHandle, pawn: u64) -> u8 {
        process.read_u8(pawn + self.offsets.pawn.team)
    }

    fn get_life_state(&self, process: &ProcessHandle, pawn: u64) -> u8 {
        process.read_u8(pawn + self.offsets.pawn.life_state)
    }

    fn get_weapon(&self, process: &ProcessHandle, pawn: u64) -> String {
        // CEntityInstance
        let weapon_entity_instance = process.read_u64(pawn + self.offsets.pawn.weapon);
        if weapon_entity_instance == 0 {
            return String::from(WEAPON_UNKNOWN);
        }
        self.get_weapon_name(process, weapon_entity_instance)
    }

    fn get_weapon_name(&self, process: &ProcessHandle, weapon_instance: u64) -> String {
        // CEntityIdentity, 0x10 = m_pEntity
        let weapon_entity_identity = process.read_u64(weapon_instance + 0x10);
        if weapon_entity_identity == 0 {
            return String::from(WEAPON_UNKNOWN);
        }
        // 0x20 = m_designerName (pointer -> string)
        let weapon_name_pointer = process.read_u64(weapon_entity_identity + 0x20);
        if weapon_name_pointer == 0 {
            return String::from(WEAPON_UNKNOWN);
        }
        process.read_string(weapon_name_pointer)
    }

    fn get_weapon_class(&self, process: &ProcessHandle, pawn: u64) -> WeaponClass {
        WeaponClass::from_string(&self.get_weapon(process, pawn))
    }

    fn is_button_down(&self, process: &ProcessHandle, button: &KeyCode) -> bool {
        // what the actual fuck is happening here?
        let value = process.read_u32(
            self.offsets.interface.input
                + (((button.u64() >> 5) * 4) + self.offsets.direct.button_state),
        );
        ((value >> (button.u64() & 31)) & 1) != 0
    }
}
