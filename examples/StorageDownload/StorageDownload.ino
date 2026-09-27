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

    String textContent = supabase.storage()
        .from("config")
        .downloadText("esp32-001/settings.json");

    if (textContent.length() > 0) {
        Serial.println("Downloaded config text:");
        Serial.println(textContent);
    } else {
        Serial.println("Failed to download config file.");
    }
}

void loop() {
}
