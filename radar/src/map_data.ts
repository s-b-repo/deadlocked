interface MapData {
    x: number;
    y: number;
    scale: number;
    lowerThreshold?: number;
}

export const MAP_DATA: Record<string, MapData> = {
    ar_baggage: {
        x: -1316,
        y: 1288,
        scale: 2.539062,
        lowerThreshold: -5,
    },
    ar_shoots: {
        x: -1368,
        y: 1952,
        scale: 2.6875,
    },
    cs_italy: {
        x: -2647,
        y: 2592,
        scale: 4.6,
    },
    cs_office: {
        x: -1838,
        y: 1858,
        scale: 4.1,
    },
    de_ancient: {
        x: -2953,
        y: 2164,
        scale: 5,
    },
    de_anubis: {
        x: -2796,
        y: 3328,
        scale: 5.22,
    },
    de_dust2: {
        x: -2476,
        y: 3239,
        scale: 4.4,
    },
    de_inferno: {
        x: -2087,
        y: 3870,
        scale: 4.9,
    },
    de_mirage: {
        x: -3230,
        y: 1713,
        scale: 5,
    },
    de_nuke: {
        x: -3453,
        y: 2887,
        scale: 7,
        lowerThreshold: -495,
    },
    de_overpass: {
        x: -4831,
        y: 1781,
        scale: 5.2,
    },
    de_train: {
        x: -2308,
        y: 2078,
        scale: 4.082077,
        lowerThreshold: -50,
    },
    de_vertigo: {
        x: -3168,
        y: 1762,
        scale: 4,
        lowerThreshold: 11700,
    },
};
