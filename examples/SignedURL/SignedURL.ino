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

    // Create a time-limited signed URL valid for 3600 seconds (1 hour)
    String signedUrl = supabase.storage()
        .from("private_assets")
        .createSignedUrl("firmware/v1.0.0.bin", 3600);

    if (signedUrl.length() > 0) {
        Serial.println("Signed URL:");
        Serial.println(signedUrl);
    } else {
        Serial.println("Failed to generate signed URL.");
    }
}

void loop() {
}
