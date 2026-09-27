#include <WiFi.h>
#include <SupabaseESP.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_KEY = "sb_publishable_xxxxxxxxx";

SupabaseESP supabase;

void setup() {
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }

    supabase.begin(SUPABASE_URL, SUPABASE_KEY);

    auto& channel = supabase.realtime().channel("room-1");

    channel.onBroadcast("robot_command", [](const JsonObjectConst& payload) {
        Serial.print("Broadcast received! Command: ");
        if (payload.containsKey("command")) {
            Serial.println(payload["command"].as<String>());
        }
    });

    channel.join();
}

uint32_t lastBroadcastMs = 0;

void loop() {
    supabase.realtime().loop();

    if (millis() - lastBroadcastMs > 10000) {
        lastBroadcastMs = millis();

        JsonDocument msg;
        msg["sender"] = "ESP32-001";
        msg["status"] = "alive";

        supabase.realtime().channel("room-1").sendBroadcast("heartbeat_signal", msg.as<JsonVariantConst>());
    }
}
