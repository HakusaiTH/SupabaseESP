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

    auto result = supabase
        .from("devices")
        .select("id,name,status")
        .eq("status", "online")
        .order("created_at", false)
        .limit(10)
        .execute();

    if (result.ok()) {
        Serial.println("Query Result:");
        Serial.println(result.body());
    } else {
        Serial.printf("Error %d: %s\n", result.statusCode(), result.errorMessage().c_str());
    }
}

void loop() {
}
