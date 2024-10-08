#ifndef WEAPONS_H
#define WEAPONS_H

enum WeaponClass {
    WEAPON_CLASS_UNKNOWN,
    WEAPON_CLASS_KNIFE,
    WEAPON_CLASS_PISTOL,
    WEAPON_CLASS_SMG,
    WEAPON_CLASS_HEAVY,   // shotguns and lmgs
    WEAPON_CLASS_RIFLE,   // all rifles except snipers
    WEAPON_CLASS_SNIPER,  // these require different handling in aimbot
    WEAPON_CLASS_GRENADE,
    WEAPON_CLASS_UTILITY,  // taser
};

#endif
