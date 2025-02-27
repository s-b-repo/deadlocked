<script lang="ts">
    import type { Player } from "./player";
    import { WeaponCategories } from "./weapons";

    export let player: Player;

    const BLUE_FILTER =
        "brightness(0) saturate(100%) invert(57%) sepia(30%) saturate(2260%) hue-rotate(195deg) brightness(97%) contrast(94%);";

    function healthColor(health: number) {
        health = Math.max(0, Math.min(100, health));

        const style = getComputedStyle(document.documentElement);
        const red = style.getPropertyValue("--color-red").trim();
        const yellow = style.getPropertyValue("--color-yellow").trim();
        const green = style.getPropertyValue("--color-green").trim();

        const hexToRgb = (hex: string): { r: number; g: number; b: number } => {
            // remove leading #
            hex = hex.slice(1);
            return {
                r: parseInt(hex.substring(0, 2), 16),
                g: parseInt(hex.substring(2, 4), 16),
                b: parseInt(hex.substring(4, 6), 16),
            };
        };

        const rgbToHex = (r: number, g: number, b: number): string =>
            "#" +
            [r, g, b]
                .map((channel) => {
                    const hex = Math.round(channel).toString(16);
                    return hex.length === 1 ? "0" + hex : hex;
                })
                .join("");

        let startColor: { r: number; g: number; b: number };
        let endColor: { r: number; g: number; b: number };
        let factor: number;

        if (health <= 50) {
            // red to yellow
            factor = health / 50;
            startColor = hexToRgb(red);
            endColor = hexToRgb(yellow);
        } else {
            // yellow to green
            factor = (health - 50) / 50;
            startColor = hexToRgb(yellow);
            endColor = hexToRgb(green);
        }

        const r = startColor.r + factor * (endColor.r - startColor.r);
        const g = startColor.g + factor * (endColor.g - startColor.g);
        const b = startColor.b + factor * (endColor.b - startColor.b);

        return rgbToHex(r, g, b);
    }
</script>

<div
    class="player gap-small"
    style:background-color={player.is_active ? 'var(--color-highlight)' : 'var(--color-base)'}
>
    <p>{player.name}</p>
    <div class="horizontal gap-small">
        {#each player.weapons.sort((a, b) => {
            const catA = WeaponCategories[a];
            const catB = WeaponCategories[b];

            if (catA === catB) {
                return a.localeCompare(b);
            }

            return catA - catB;
        }) as weapon}
            <img src={`/icons/${weapon}.svg`} alt={weapon} style="filter: {weapon === player.weapon ? BLUE_FILTER : ''}" />
        {/each}
    </div>
    <div class="progress-container">
        <div class="progress" style:width="{player.health}%" style:background-color={healthColor(player.health)}></div>
    </div>
    <div class="progress-container">
        <div class="progress" style:width="{player.armor}%" style:background-color="var(--color-blue)"></div>
    </div>
</div>

<style>
    .player {
        border: var(--border-text);
        border-radius: 0.5rem;
        padding: 0.5rem;
        display: flex;
        flex-direction: column;
        align-items: center;
        font-size: var(--font-size-medium);
        width: calc(100% - 1rem - 4px);
    }

    .horizontal {
        display: flex;
        flex-direction: row;
        gap: 0.2rem;
    }

    .gap-small {
        gap: 0.5rem;
    }

    img {
        max-width: 4rem;
        max-height: 2rem;
        transition: none;
    }

    p {
        margin: 0;
    }

    .progress-container {
        height: 0.5rem;
        width: 90%;
        border-radius: 0.5rem;
        background-color: var(--color-highlight);
    }

    .progress {
        height: 100%;
        border-radius: 0.5rem;
    }
</style>
