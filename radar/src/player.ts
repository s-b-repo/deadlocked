export interface Player {
    name: string;
    health: number;
    armor: number;
    position: Vec3;
    rotation: number;
    team: number;
    weapon: string;
    weapons: string[];
    is_active: boolean;
}

export interface Vec3 {
    x: number;
    y: number;
    z: number;
}

export interface Vec2 {
    x: number;
    y: number;
}

export enum Team {
    Spectator = 1,
    T = 2,
    CT = 3,
}
