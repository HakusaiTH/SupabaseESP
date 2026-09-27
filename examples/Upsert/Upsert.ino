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

    JsonDocument doc;
    doc["device_id"] = "ESP32-001";
    doc["last_seen"] = "2026-09-28T00:00:00Z";
    doc["ip_address"] = "192.168.1.50";

    auto result = supabase
        .from("device_state")
        .upsert(doc, "device_id")
        .execute();

    if (result.ok()) {
        Serial.println("Upsert successful!");
    } else {
        Serial.printf("Upsert error [%d]: %s\n", result.statusCode(), result.errorMessage().c_str());
    }
}

void loop() {
}
