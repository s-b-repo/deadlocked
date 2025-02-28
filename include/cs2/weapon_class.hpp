#pragma once

#include <string>

enum class WeaponClass {
    Unknown,
    Knife,
    Pistol,
    Smg,
    Heavy,  // shotguns and lmgs
    Shotgun,
    Rifle,   // all rifles except snipers
    Sniper,  // these require different handling in aimbot
    Grenade,
    Utility  // taser
};

WeaponClass WeaponClassFromString(const std::string &name);
