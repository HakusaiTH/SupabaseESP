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
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected.");

    supabase.begin(SUPABASE_URL, SUPABASE_KEY);

    JsonDocument doc;
    doc["device_id"] = "ESP32-001";
    doc["temperature"] = 28.5;
    doc["humidity"] = 61.2;

    auto result = supabase
        .from("telemetry")
        .insert(doc)
        .select("*")
        .execute();

    if (result.ok()) {
        Serial.println("Inserted row representation:");
        Serial.println(result.body());
    } else {
        Serial.printf("Insert failed [%d]: %s\n", result.statusCode(), result.errorMessage().c_str());
    }
}

void loop() {
}
