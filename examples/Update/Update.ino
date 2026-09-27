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
    doc["status"] = "offline";
    doc["battery"] = 85;

    auto result = supabase
        .from("devices")
        .update(doc)
        .eq("device_id", "ESP32-001")
        .execute();

    if (result.ok()) {
        Serial.println("Update successful!");
    } else {
        Serial.printf("Update error [%d]: %s\n", result.statusCode(), result.errorMessage().c_str());
    }
}

void loop() {
}
