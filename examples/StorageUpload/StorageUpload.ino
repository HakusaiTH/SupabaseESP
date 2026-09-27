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

    String payload = "{\"device\": \"ESP32-001\", \"status\": \"booted\"}";

    auto result = supabase.storage()
        .from("telemetry_files")
        .upload("esp32-001/status.json", payload, "application/json", true);

    if (result.ok()) {
        Serial.println("Storage upload successful!");
    } else {
        Serial.printf("Storage upload failed [%d]: %s\n", result.statusCode(), result.errorMessage().c_str());
    }
}

void loop() {
}
