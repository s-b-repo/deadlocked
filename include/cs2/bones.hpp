#pragma once

#include <vector>

enum class Bones {
    Pelvis = 0,
    Spine1 = 2,
    Spine2 = 4,
    Neck = 5,
    Head = 6,
    LeftShoulder = 8,
    LeftElbow = 9,
    LeftHand = 10,
    RightShoulder = 13,
    RightElbow = 14,
    RightHand = 15,
    LeftHip = 22,
    LeftKnee = 23,
    LeftFoot = 24,
    RightHip = 25,
    RightKnee = 26,
    RightFoot = 27
};

const std::vector<Bones> all_bones = {
    Bones::Pelvis,       Bones::Spine1,    Bones::Spine2,   Bones::Neck,          Bones::Head,
    Bones::LeftShoulder, Bones::LeftElbow, Bones::LeftHand, Bones::RightShoulder, Bones::RightElbow,
    Bones::RightHand,    Bones::LeftHip,   Bones::LeftKnee, Bones::LeftFoot,      Bones::RightHip,
    Bones::RightKnee,    Bones::RightFoot};

const std::vector<std::pair<Bones, Bones>> bone_connections = {
    {Bones::Pelvis, Bones::Spine1},
    {Bones::Spine1, Bones::Spine2},
    {Bones::Spine2, Bones::Neck},
    {Bones::Neck, Bones::Head},
    {Bones::Neck, Bones::LeftShoulder},
    {Bones::LeftShoulder, Bones::LeftElbow},
    {Bones::LeftElbow, Bones::LeftHand},
    {Bones::Neck, Bones::RightShoulder},
    {Bones::RightShoulder, Bones::RightElbow},
    {Bones::RightElbow, Bones::RightHand},
    {Bones::Pelvis, Bones::LeftHip},
    {Bones::LeftHip, Bones::LeftKnee},
    {Bones::LeftKnee, Bones::LeftFoot},
    {Bones::Pelvis, Bones::RightHip},
    {Bones::RightHip, Bones::RightKnee},
    {Bones::RightKnee, Bones::RightFoot},
};
