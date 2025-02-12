#pragma once

#include <vector>

enum Bones {
    BonePelvis = 0,
    BoneSpine1 = 2,
    BoneSpine2 = 4,
    BoneNeck = 5,
    BoneHead = 6,
    BoneLeftShoulder = 8,
    BoneLeftElbow = 9,
    BoneLeftHand = 10,
    BoneRightShoulder = 13,
    BoneRightElbow = 14,
    BoneRightHand = 15,
    BoneLeftHip = 22,
    BoneLeftKnee = 23,
    BoneLeftFoot = 24,
    BoneRightHip = 25,
    BoneRightKnee = 26,
    BoneRightFoot = 27
};

const std::vector<Bones> all_bones = {
    BonePelvis,    BoneSpine1,   BoneSpine2,        BoneNeck,       BoneHead,      BoneLeftShoulder,
    BoneLeftElbow, BoneLeftHand, BoneRightShoulder, BoneRightElbow, BoneRightHand, BoneLeftHip,
    BoneLeftKnee,  BoneLeftFoot, BoneRightHip,      BoneRightKnee,  BoneRightFoot};

const std::vector<std::pair<Bones, Bones>> bone_connections = {
    {Bones::BonePelvis, Bones::BoneSpine1},
    {Bones::BoneSpine1, Bones::BoneSpine2},
    {Bones::BoneSpine2, Bones::BoneNeck},
    {Bones::BoneNeck, Bones::BoneHead},
    {Bones::BoneNeck, Bones::BoneLeftShoulder},
    {Bones::BoneLeftShoulder, Bones::BoneLeftElbow},
    {Bones::BoneLeftElbow, Bones::BoneLeftHand},
    {Bones::BoneNeck, Bones::BoneRightShoulder},
    {Bones::BoneRightShoulder, Bones::BoneRightElbow},
    {Bones::BoneRightElbow, Bones::BoneRightHand},
    {Bones::BonePelvis, Bones::BoneLeftHip},
    {Bones::BoneLeftHip, Bones::BoneLeftKnee},
    {Bones::BoneLeftKnee, Bones::BoneLeftFoot},
    {Bones::BonePelvis, Bones::BoneRightHip},
    {Bones::BoneRightHip, Bones::BoneRightKnee},
    {Bones::BoneRightKnee, Bones::BoneRightFoot},
};
