#include "cs2/player.hpp"

#include "cs2/cs2.hpp"

std::optional<Player> Player::LocalPlayer() {
    const u64 controller = process.Read<u64>(offsets.direct.local_player);
    if (controller == 0) {
        return std::nullopt;
    }

    const auto pawn = Pawn(controller);
    if (!pawn.has_value()) {
        return std::nullopt;
    }
    return Player{.controller = controller, .pawn = pawn.value()};
}

std::optional<Player> Player::Index(u64 index) {
    // wtf is this doing, and how?
    const u64 v1 = process.Read<u64>(offsets.interface.entity + 0x08 * (index >> 9) + 0x10);
    if (v1 == 0) {
        return std::nullopt;
    }
    // what?
    const u64 controller = process.Read<u64>(v1 + 120 * (index & 0x1ff));
    if (controller == 0) {
        return std::nullopt;
    }

    const auto pawn = Pawn(controller);
    if (!pawn.has_value()) {
        return std::nullopt;
    }
    return Player{.controller = controller, .pawn = pawn.value()};
}

std::optional<u64> Player::Pawn(u64 controller) {
    const u64 v1 = process.Read<i32>(controller + offsets.controller.pawn);
    if (v1 == -1) {
        return std::nullopt;
    }

    // what the fuck is this doing?
    const u64 v2 = process.Read<u64>(offsets.interface.player + 8 * ((v1 & 0x7fff) >> 9));
    if (v2 == 0) {
        return std::nullopt;
    }

    // bit-fuckery, why is this needed exactly?
    const u64 entity = process.Read<u64>(v2 + 120 * (v1 & 0x1ff));
    if (entity == 0) {
        return std::nullopt;
    }

    return entity;
}

i32 Player::Health() {
    const i32 health = process.Read<i32>(pawn + offsets.pawn.health);
    if (health < 0 || health > 100) {
        return 0;
    }
    return health;
}

i32 Player::Armor() {
    const i32 armor = process.Read<i32>(pawn + offsets.pawn.armor);
    if (armor < 0 || armor > 100) {
        return 0;
    }
    return armor;
}

std::string Player::Name() {
    const u64 name_address = process.Read<u64>(controller + offsets.controller.name);
    if (name_address == 0) {
        return std::string("?");
    }

    return process.ReadString(name_address);
}

u8 Player::Team() { return process.Read<u8>(pawn + offsets.pawn.team); }

u8 Player::LifeState() { return process.Read<u8>(pawn + offsets.pawn.life_state); }

std::string Player::WeaponName() {
    // CEntityInstance
    const u64 weapon_entity_instance = process.Read<u64>(pawn + offsets.pawn.weapon);
    if (weapon_entity_instance == 0) {
        return std::string("?");
    }
    // CEntityIdentity, 0x10 = m_pEntity
    const u64 weapon_entity_identity = process.Read<u64>(weapon_entity_instance + 0x10);
    if (weapon_entity_identity == 0) {
        return std::string("?");
    }
    // 0x20 = m_designerName (pointer -> string)
    const u64 weapon_name_pointer = process.Read<u64>(weapon_entity_identity + 0x20);
    if (weapon_name_pointer == 0) {
        return std::string("?");
    }

    auto name = process.ReadString(weapon_name_pointer);
    if (name.find("weapon_") != std::string::npos) {
        name = name.substr(7);
    }
    return name;
}

WeaponClass Player::GetWeaponClass() {
    const auto name = WeaponName();
    return WeaponClassFromString(name);
}

u64 Player::GameSceneNode() { return process.Read<u64>(pawn + offsets.pawn.game_scene_node); }

bool Player::IsDormant() {
    const u64 gs_node = GameSceneNode();
    return process.Read<u8>(gs_node + offsets.game_scene_node.dormant) != 0;
}

glm::vec3 Player::Position() {
    const u64 gs_node = GameSceneNode();
    return process.Read<glm::vec3>(gs_node + offsets.game_scene_node.origin);
}

glm::vec3 Player::EyePosition() {
    const glm::vec3 position = Position();
    const glm::vec3 eye_offset = process.Read<glm::vec3>(pawn + offsets.pawn.eye_offset);

    return position + eye_offset;
}

glm::vec3 Player::BonePosition(u64 bone_index) {
    const u64 gs_node = GameSceneNode();
    const u64 bone_data = process.Read<u64>(gs_node + offsets.game_scene_node.model_state + 0x80);

    if (bone_data == 0) {
        return glm::vec3(0.0);
    }

    return process.Read<glm::vec3>(bone_data + (bone_index * 32));
}

i32 Player::ShotsFired() { return process.Read<i32>(pawn + offsets.pawn.shots_fired); }

f32 Player::FovMultiplier() { return process.Read<f32>(pawn + offsets.pawn.fov_multiplier); }

u64 Player::SpottedMask() { return process.Read<u64>(pawn + offsets.pawn.spotted_state + offsets.spotted_state.mask); }

std::vector<std::pair<glm::vec3, glm::vec3>> Player::AllBones() {
    std::unordered_map<Bones, glm::vec3> bones;

    for (const Bones bone : all_bones) {
        const glm::vec3 position = BonePosition(bone);
        bones.insert({bone, position});
    }

    std::vector<std::pair<glm::vec3, glm::vec3>> connections(bone_connections.size());

    i32 i = 0;
    for (const auto connection : bone_connections) {
        connections[i] = {bones.at(connection.first), bones.at(connection.second)};
        i++;
    }

    return connections;
}

bool Player::IsValid() {
    if (IsDormant()) {
        return false;
    }

    if (Health() <= 0) {
        return false;
    }

    if (LifeState() != 0) {
        return false;
    }

    return true;
}

bool Player::IsFlashed() { return process.Read<f32>(pawn + offsets.pawn.flash_duration) > 0.2f; }

void Player::NoFlash(f32 max_alpha) {
    if (max_alpha < 0.0f) {
        max_alpha = 0.0f;
    } else if (max_alpha > 255.0f) {
        max_alpha = 255.0f;
    }
    if (process.Read<f32>(pawn + offsets.pawn.flash_alpha) != max_alpha) {
        process.Write<f32>(pawn + offsets.pawn.flash_alpha, max_alpha);
    }
}

void Player::SetFov(i32 fov) {
    u64 camera_service = process.Read<u64>(pawn + offsets.pawn.camera_services);
    if (camera_service == 0) {
        return;
    }
    if (process.Read<u32>(camera_service + offsets.camera_service.fov) != fov) {
        process.Write<i32>(controller + offsets.controller.desired_fov, fov);
    }
}

glm::vec2 Player::ViewAngles() { return process.Read<glm::vec2>(pawn + offsets.pawn.view_angles); }

glm::vec2 Player::AimPunch() {
    const u64 length = process.Read<u64>(pawn + offsets.pawn.aim_punch_cache);
    if (length < 1) {
        return glm::vec2(0.0);
    }

    const u64 data_address = process.Read<u64>(pawn + offsets.pawn.aim_punch_cache + 0x08);

    return process.Read<glm::vec2>(data_address + (length - 1) * 12) * glm::vec2(2.0);
}
