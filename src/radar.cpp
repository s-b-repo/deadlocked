#include "radar.hpp"

#include <hv/EventLoop.h>
#include <hv/WebSocketClient.h>

#include <chrono>
#include <string>
#include <thread>

#include "hv/hloop.h"
#include "log.hpp"

class RadarWebSocket : public hv::WebSocketClient {
  public:
    std::string uuid;

    RadarWebSocket(hv::EventLoopPtr loop = nullptr) : hv::WebSocketClient(loop) {}
    ~RadarWebSocket() {}

    void connect(std::string url) {
        onopen = [this]() {
            Log(LogLevel::Debug, "websocket connection opened");
            send(R"({"type": "server"})");
        };

        onmessage = [this](const std::string &message) {
            Log(LogLevel::Debug, "message: " + message);

            // is valid uuid length (32 hex digits + 4 dashes)
            if (message.length() == 36) {
                uuid = message;
                Log(LogLevel::Info, "radar uuid received: " + uuid);
            }
        };

        onclose = [this]() { Log(LogLevel::Debug, "websocket connection closed"); };

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

void Radar() {
    RadarWebSocket ws;
    ws.connect("ws://localhost:5000");
    // if this is not here the thread just dies
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
