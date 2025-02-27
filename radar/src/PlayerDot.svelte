<script lang="ts">
    import type { Player, Vec2 } from "./player";

    export let player: Player;
    export let position: Vec2;
    export let friendly: boolean;
    export let sameLevel: boolean;
</script>

<svg
    xmlns="http://www.w3.org/2000/svg"
    width="24"
    height="24"
    viewBox="0 0 24 24"
    style:transform="rotate({-player.rotation + 90}deg)"
    style:left="calc({position.x}px - 1.25%)"
    style:top="calc({-position.y}px - 1.25%)"
    style:opacity={sameLevel ? 1 : 0.5}
>
    <!--fucking kill me for this horrible nested ternary ._.-->
    <path
        fill={player.is_active ? "var(--color-green)" : friendly ? "var(--color-blue)" : "var(--color-red)"}
        d="M10.708 2.372a2.382 2.382 0 0 0 -.71 .686l-4.892 7.26c-1.981 3.314 -1.22 7.466 1.767 9.882c2.969 2.402 7.286 2.402 10.254 0c2.987 -2.416 3.748 -6.569 1.795 -9.836l-4.919 -7.306c-.722 -1.075 -2.192 -1.376 -3.295 -.686z"
    />
</svg>

<style>
    svg {
        transition: 0.02s;
        position: absolute;
        width: 2.5%;
        height: 2.5%;
        transform-origin: 50% 60%;
        filter: drop-shadow(0 0 4px #000000);
    }
</style>
