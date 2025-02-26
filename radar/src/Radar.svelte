<script lang="ts">
    import { onMount } from "svelte";
    import { MAP_DATA } from "./map_data";
    import type { Player } from "./player";
    import PlayerDot from "./PlayerDot.svelte";

    let ws: WebSocket | null = null;
    let wsConnected = false;
    let wsTimer: number | null = null;

    let radar: HTMLDivElement | undefined = $state();

    const gameInfo = $state({ uuid: "", map: "de_dust2", lowerRadar: false, mapData: MAP_DATA["de_dust2"] });

    let players: Player[] = $state([]);
    let activePlayer: Player | undefined = $state();

    function startWS() {
        stopWS();

        // get address from window.location
        const url = window.location.origin.replace("http://", "ws://");
        ws = new WebSocket(url);
        ws.onmessage = wsMessage;
        ws.onopen = () => {
            wsConnected = true;
            console.info("websocket connection established");
            ws?.send(JSON.stringify({ type: "client", uuid: gameInfo.uuid ?? "" }));
            wsTimer = setInterval(() => {
                ws?.send(JSON.stringify({ type: "client", uuid: gameInfo.uuid ?? "" }));
            }, 50);
        };
        ws.onclose = () => {
            wsConnected = false;
            if (wsTimer) {
                clearInterval(wsTimer);
            }
            console.info("websocket connection closed");
        };
        ws.onerror = () => {
            wsConnected = false;
            if (wsTimer) {
                clearInterval(wsTimer);
            }
            console.info("websocket could not connect");
        };
    }

    function stopWS() {
        ws?.close();
        ws = null;
    }

    /** @param {MessageEvent} event */
    function wsMessage(event: MessageEvent) {
        const data = JSON.parse(event.data);

        players = data["players"] ?? [];
        activePlayer = players.find((player) => player.is_active);
        if (!activePlayer) {
            return;
        }

        const map: string = data["map"] ?? "de_dust2";
        if (map !== gameInfo.map) {
            // update radar image
            gameInfo.map = map.length === 0 ? "de_dust2" : map;
            gameInfo.mapData = MAP_DATA[gameInfo.map];
        }

        if (MAP_DATA[map].lowerThreshold && activePlayer.position.z < MAP_DATA[map].lowerThreshold) {
            gameInfo.lowerRadar = true;
        } else {
            gameInfo.lowerRadar = false;
        }
    }

    onMount(() => {
        gameInfo.uuid = new URLSearchParams(window.location.search).get("game") ?? "";
        if (gameInfo.uuid.length !== 36) {
            console.error("no game id query parameter");
        } else {
            console.info(`game id: ${gameInfo.uuid}`);
        }

        startWS();

        setInterval(() => {
            if (ws && ws.readyState !== 1) {
                startWS();
            }
        }, 5000);
    });
</script>

<main>
    <div
        bind:this={radar}
        id="radar"
        style:background-image={`url(/images/${gameInfo.map}${gameInfo.lowerRadar ? "_lower" : ""}.png)`}
    >
        {#each players.filter((player) => player.health > 0) as player}
            <PlayerDot
                {player}
                position={{
                    x:
                        ((player.position.x - gameInfo.mapData.x) / gameInfo.mapData.scale) *
                        (radar?.clientWidth / 1024),
                    y:
                        ((player.position.y - gameInfo.mapData.y) / gameInfo.mapData.scale) *
                        (radar?.clientHeight / 1024),
                }}
                friendly={player.team === activePlayer?.team}
                sameLevel={gameInfo.lowerRadar ===
                    (gameInfo.mapData.lowerThreshold && player.position.z < gameInfo.mapData.lowerThreshold)}
            />
        {/each}
    </div>
</main>

<style>
    main {
        height: 100%;
        display: flex;
        align-items: center;
        justify-content: center;
    }

    #radar {
        height: 90dvh;
        aspect-ratio: 1/1;
        border: var(--border-text);
        position: relative;
        border-radius: 0.5rem;
        background-size: cover;
    }
</style>
