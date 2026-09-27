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

    auto& channel = supabase.realtime().channel("db-changes");

    channel.onPostgresChange(
        SupabaseChangeEvent::Insert,
        "public",
        "devices",
        [](const SupabasePostgresChange& change) {
            Serial.println("New row inserted in 'devices' table:");
            if (change.record.is<JsonObject>()) {
                String recordJson;
                serializeJson(change.record, recordJson);
                Serial.println(recordJson);
            }
        }
    );

    channel.join();
}

void loop() {
    supabase.realtime().loop();
}
