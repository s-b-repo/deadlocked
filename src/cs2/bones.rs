#![allow(unused)]

use strum::EnumIter;

#[derive(Debug, EnumIter)]
enum Bones {
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
    RightFoot = 27,
}
