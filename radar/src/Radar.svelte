<script lang="ts">
    import { onMount } from "svelte";
    import { MAP_DATA } from "./map_data";
    import { Team, type Player } from "./player";
    import PlayerDot from "./PlayerDot.svelte";
    import PlayerInfo from "./PlayerInfo.svelte";

    let ws: WebSocket | null = null;
    let wsConnected = $state(false);
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
    <svg
        xmlns="http://www.w3.org/2000/svg"
        width="24"
        height="24"
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        stroke-width="2"
        stroke-linecap="round"
        stroke-linejoin="round"
        class={wsConnected ? "connected" : "disconnected"}
    >
        <path d="M7 12l5 5l-1.5 1.5a3.536 3.536 0 1 1 -5 -5l1.5 -1.5z" />
        <path d="M17 12l-5 -5l1.5 -1.5a3.536 3.536 0 1 1 5 5l-1.5 1.5z" />
        <path d="M3 21l2.5 -2.5" />
        <path d="M18.5 5.5l2.5 -2.5" />
        <path d="M10 11l-2 2" />
        <path d="M13 14l-2 2" />
    </svg>

    <div class="player-info">
        <h1 id="t">T</h1>
        {#each players.filter((player) => player.team === Team.T) as player}
            <PlayerInfo {player} />
        {/each}
    </div>

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

    <div class="player-info">
        <h1 id="ct">CT</h1>
        {#each players.filter((player) => player.team === Team.CT) as player}
            <PlayerInfo {player} />
        {/each}
    </div>
</main>

<style>
    main {
        height: 100%;
        display: flex;
        flex-direction: row;
        align-items: center;
        justify-content: space-evenly;
    }

    h1 {
        margin: 0;
    }

    #radar {
        width: 90dvh;
        aspect-ratio: 1 / 1;
        border: var(--border-text);
        position: relative;
        border-radius: 0.5rem;
        background-size: cover;
    }

    .player-info {
        display: flex;
        flex-direction: column;
        gap: 1rem;
        min-width: 20rem;
        margin: 0 1rem;
        text-align: center;
    }

    #t,
    #ct {
        border-radius: 0.5rem;
        color: var(--color-base);
        padding: 0.5rem;
    }

    #t {
        background-color: var(--color-orange);
    }

    #ct {
        background-color: var(--color-blue);
    }

    svg {
        position: fixed;
        top: 1rem;
        left: 50%;
        width: 2rem;
        height: 2rem;
        border: var(--border-text);
        border-radius: 0.5rem;
        transform: translateX(-50%);
    }

    svg.connected {
        stroke: var(--color-green);
    }

    svg.disconnected {
        stroke: var(--color-red);
    }

    @media (max-aspect-ratio: 1 / 1) {
        #radar {
            width: 90%;
        }

        .player-info {
            display: none;
        }
    }
</style>
