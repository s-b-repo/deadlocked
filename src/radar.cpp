#include "radar.hpp"

#include <hv/EventLoop.h>
#include <hv/WebSocketClient.h>

#include <chrono>
#include <thread>

#include "globals.hpp"
#include "hv/hloop.h"
#include "json.hpp"
#include "log.hpp"

std::string uuid;
bool radar_connected;

class RadarWebSocket : public hv::WebSocketClient {
  public:
    RadarWebSocket(hv::EventLoopPtr loop = nullptr) : hv::WebSocketClient(loop) {}
    ~RadarWebSocket() {}

    void connect(const std::string &url) {
        onopen = [this]() {
            Log(LogLevel::Debug, "websocket connection opened");
            send(R"({"type": "server"})");
            radar_connected = true;
        };

        // only message received should be id given to game
        onmessage = [this](const std::string &message) {
            Log(LogLevel::Debug, "message: " + message);

            // is valid uuid length (32 hex digits + 4 dashes)
            if (message.length() == 36) {
                uuid = message;
                Log(LogLevel::Info, "radar uuid received: " + uuid);
            }
        };

        onclose = [this]() {
            Log(LogLevel::Debug, "websocket connection closed");
            radar_connected = false;
        };

        setPingInterval(10000);

        reconn_setting_t reconnect;
        reconn_setting_init(&reconnect);
        reconnect.min_delay = 1000;
        reconnect.max_delay = 10000;
        reconnect.delay_policy = 2;
        setReconnect(&reconnect);

        open(url.c_str());
    }
};

RadarWebSocket ws;

[[noreturn]] void Radar() {
    ws.connect(config.misc.radar_url);
    // if this is not here the thread just dies
    while (true) {
        config_lock.lock();
        if (ws.url != config.misc.radar_url) {
            ws.close();
            ws.open(config.misc.radar_url.c_str());
        }
        config_lock.unlock();

        if (ws.isConnected()) {
            nlohmann::json json;

            vinfo_lock.lock();
            for (const PlayerInfo &player : all_player_info) {
                json["data"]["players"].push_back(
                    {{"name", player.name},
                     {"health", player.health},
                     {"armor", player.armor},
                     {"position",
                      {{"x", player.position.x},
                       {"y", player.position.y},
                       {"z", player.position.z}}},
                     {"rotation", player.rotation},
                     {"team", player.team},
                     {"weapon", player.weapon},
                     {"weapons", player.weapons},
                     {"is_active", player.is_active}});
            }
            json["data"]["map"] = misc_info.in_game ? misc_info.map_name : "";
            vinfo_lock.unlock();

            json["type"] = "data";
            json["uuid"] = uuid;

            ws.send(json.dump());
        } else {
            ws.open(ws.url.c_str());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
