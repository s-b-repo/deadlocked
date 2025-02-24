import express from "express";
import expressWs from "express-ws";
import path from "path";
import { fileURLToPath } from "url";
import { v4 as uuidV4 } from "uuid";

const PORT = 5000;

const app = express();
expressWs(app);

const filename = fileURLToPath(import.meta.url);
const dirname = path.dirname(filename);

/** @type {Record<string, Object>} */
const games = {};

app.get("/", (req, res) => {
    res.sendFile(path.join(dirname, "radar.html"));
});

app.ws("/", (ws, req) => {
    console.info("new websocket connection established");

    ws.on("message", (message) => {
        console.info(`received websocket message: ${message}`);
        const content = JSON.parse(message);

        if (content["type"] === "server") {
            // server setup
            const id = uuidV4();
            games[id] = {};
            ws.send(id);
        } else if (content["type"] === "data") {
            // server data
            const id = content["uuid"];
            if (id in games) {
                games[id] = content["data"];
            }
        } else if (content["type"] === "client") {
            // client data request
            const id = content["uuid"];
            if (id && id in games) {
                ws.send(JSON.stringify(games[id]));
            } else {
                ws.send("not found");
            }
        }
    });

    ws.on("close", () => {
        console.info("closing connection");
    });
});

app.listen(PORT, () => {
    console.info(`server started on port ${PORT}`);
});
